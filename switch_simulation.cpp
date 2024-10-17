#include "switch_simulation.h"
#include "fmt/format.h"

#include <utility>
template <typename F>
struct OnExit {
	F func;
	explicit OnExit(F&& f)
	    : func(std::forward<F>(f)) {
	}
	~OnExit() {
		if (doNotCall) {
			return;
		}
		func();
	}
	//do not call any longer
	void reset() {
		doNotCall = true;
	}

      private:
	bool doNotCall = false;
};

template <typename... T>
[[nodiscard]] QString F16(const std::string_view& fmt, T&&... args) {
	return QString::fromStdString(fmt::vformat(fmt, fmt::make_format_args(args...)));
}

void Input::toggle() {
	set(!state);
}

bool Input::set(bool neu) {
	if (neu == state) {
		return false;
	}
	state = neu;
	edge  = true;
	return true;
}

void Input::on() {
	set(true);
}

void Input::off() {
	set(false);
}

QString Input::getState() const {
	return QString("%1    %2").arg(state ? "True " : "False").arg(edge ? "True" : "False");
}

QString Switch::getState() {
	return state ? "ON" : "OFF";
}

void Switch::resetEdge() {
	inputs.physicalSwitch.edge = false;
	inputs.webButton.edge      = false;
	inputs.timerForced.edge    = false;
	inputs.awayFromHome.edge   = false;
	inputs.timerGreen.edge     = false;
	inputs.excessGreen.edge    = false;
}

void Switch::resetPrimary() {
	inputs.physicalSwitch.primary = false;
	inputs.webButton.primary      = false;
	inputs.timerForced.primary    = false;
	inputs.awayFromHome.primary   = false;
	inputs.timerGreen.primary     = false;
	inputs.excessGreen.primary    = false;
}

bool Switch::determineNextState() {
	OnExit oe([&]() {
		resetEdge();
	});
	// Highest priority: Physical Switch
	if (inputs.physicalSwitch.edge) {
		inputs.physicalSwitch.primary = true;
		reason                        = "Physical Switch was Toggled";
		return inputs.physicalSwitch.state;
	}

	// Web Button Toggle
	if (inputs.webButton.edge) {
		//this is the only way to bypass the phisical switch
		resetPrimary();
		inputs.webButton.primary = true;
		reason                   = "Web Button was Toggled";
		return !state;
	}

	//we can not turn off from this state with the lower prio event
	if (state && inputs.physicalSwitch.state && inputs.physicalSwitch.primary) {
		reason = "System remains ON due to phisical switch";
		return state;
	}

	// Timer Forced
	if (inputs.timerForced.edge) {
		if (inputs.timerForced.state) {
			if (state) { //if already ON, don't turn ON again
				reason = "Timer started, but device was already ON";
				return state;
			}
			reason = "Timer started";
			resetPrimary();
			inputs.timerForced.primary = true;
			inputs.timerForced.used    = true;
			return true;
		}
		if (inputs.timerForced.used) {
			reason                     = "Timer expired and was used";
			inputs.timerForced.primary = false;
			return false;
		}
		reason = "Timer expired but was NOT used, we leave same state";
		return state;
	}

	if (inputs.awayFromHome.edge) {
		if (inputs.awayFromHome.state) {
			if (state && inputs.excessGreen.primary) {
				reason = "Away from home, user asked to not manage the device, else energy is wasted";
				return false;
			}
		} else {
			if (!state && inputs.excessGreen.primary) {
				reason = "User got back home, we still have green energy to send to him";
				return true;
			}
		}
	}

	//If we have green energy we have 2 condition, a list of timers and a remote command (that is a special case of timer)
	//we intentionally do not check edge, as is ok if was emitted before
	if (greenOk()) {
		if (inputs.excessGreen.primary) {
			reason = "We are already using green energy";
			return state;
		}
		if (state) {
			reason = "Green Energy available, but device is already ON";

			if (inputs.timerForced.primary) {
				//we can run green after a timer expires
			} else {
				inputs.excessGreen.used = true;
			}

			return state;
		}
		if (inputs.awayFromHome.state) {
			reason                  = "Away from home, user asked to not manage the device, else energy is wasted";
			inputs.excessGreen.used = true;
			return state;
		}
		if (inputs.timerGreen.state) {
			if (inputs.excessGreen.used) {
				reason = "Green Energy available, but event was already used and stopped by user, so we do not turn on again. We leave same state";
				return state;
			}
			inputs.excessGreen.primary = true;
			inputs.excessGreen.used    = true;
			reason                     = "Green Energy available and timer is active, let's GO!";
			return true;
		}
		reason = "Green Energy available, but timer is not active, we leave same state";
		return state;
	}

	if (inputs.excessGreen.primary) {
		reason                     = "Green Energy was used, but now is not available any longer, we turn off";
		inputs.excessGreen.used    = false;
		inputs.excessGreen.primary = false;
		return false;
	}

	if (inputs.excessGreen.edge) {
		if (!inputs.excessGreen.state) {
			inputs.excessGreen.used    = false;
			inputs.excessGreen.primary = false;
		}
		return state;
	}

	// Default: Remain in current state
	return state;
}

bool Switch::next() {
	bool oldState = state;
	state         = determineNextState();

	if (oldState != state) {
		emit stateChanged(state);
		emit debugMessage(QString("Transition: %1 -> %2\n%3")
		                      .arg(oldState ? "ON" : "OFF")
		                      .arg(state ? "ON" : "OFF")
		                      .arg(reason));
	} else {
		emit debugMessage(QString("Stable: %1\n%2")
		                      .arg(state ? "ON" : "OFF")
		                      .arg(reason));
	}

	inputs.webButton.state = state;
	reason.clear();

	return state;
}

//in certain condition a change trigger a series of event, we keep simulating until we reach a stable state
void Switch::multiStep() {
	while (true) {
		auto old = state;
		next();
		if (old == state) {
			return;
		}
	}
}

bool Switch::greenOk() const {
	return inputs.excessGreen.state && inputs.timerGreen.state;
}
