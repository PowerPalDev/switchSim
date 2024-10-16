// switch_simulation.h

#ifndef HOME_ROY_PUBLIC2_SWITCHSIM_SWITCH_SIMULATION_H
#define HOME_ROY_PUBLIC2_SWITCHSIM_SWITCH_SIMULATION_H

#include <QObject>
#include <QString>

// Inputs struct
struct Input {
	QString name;
	explicit Input(const QString& name_)
	    : name(name_) {};

	bool state   = false; // Current state
	bool edge    = false; // Edge detected
	bool primary = false;
	bool used    = false;

	void toggle();

	/**
	 * @brief set
	 * @param neu
	 * @return if changed
	 */
	bool set(bool neu);

	void    on();
	void    off();
	QString getState() const;
};

// Inputs struct
struct Inputs {
	Input physicalSwitch{"Physical Switch"};
	Input webButton{"Web Button"};
	Input timerForced{"Timer Forced"};
	Input awayFromHome{"Away From Home"};
	Input timerGreen{"Timer Green"};
	Input excessGreen{"Excess Green"};
};

class Switch : public QObject {
	Q_OBJECT
      public:
	explicit Switch(QObject* parent = nullptr)
	    : QObject(parent) {
	}

	bool    state = false;
	QString reason;
	Inputs  inputs;

	QString getState();

	void resetEdge();

	void resetPrimary();

	// Function to determine the next state based on current state and inputs
	bool determineNextState();

	bool next();

	QString getInputs();

	void multiStep();

      signals:
	void stateChanged(bool state);
	void debugMessage(const QString& message);

      private:
};

#endif // HOME_ROY_PUBLIC2_SWITCHSIM_SWITCH_SIMULATION_H
