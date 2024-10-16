#include <QApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollBar>
#include <QString>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <iostream>
#include <sstream>

// Include the simulation code (modified as necessary)
#include "switch_simulation.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

	// Create the main window widget
	QWidget* window = new QWidget;
	window->setWindowTitle("Switch Simulation UI");

	QPushButton* clear = new QPushButton("clear");

	// Create the debug text area
	QTextEdit* debugTextArea = new QTextEdit;
	debugTextArea->setReadOnly(true);
	debugTextArea->setPlaceholderText("Debug output will appear here...");

	// Create the input buttons
	QPushButton* physicalSwitch = new QPushButton("Physical Switch");
	QPushButton* webButton      = new QPushButton("Web Button");
	QPushButton* timerForced    = new QPushButton("Timer Forced");
	QPushButton* awayFromHome   = new QPushButton("Away From Home");
	QPushButton* timerGreen     = new QPushButton("Timer Green");
	QPushButton* excessGreen    = new QPushButton("Excess Green");

	// Make buttons checkable
	physicalSwitch->setCheckable(true);
	webButton->setCheckable(true);
	timerForced->setCheckable(true);
	awayFromHome->setCheckable(true);
	timerGreen->setCheckable(true);
	excessGreen->setCheckable(true);

	// Function to change button color when toggled
	auto changeButtonColor = [](QPushButton* button, bool checked) {
		if (checked)
			button->setStyleSheet("background-color: green;");
		else
			button->setStyleSheet("");
	};

	// Create the output button (lamp indicator)
	QPushButton* outputButton = new QPushButton("Off");
	outputButton->setCheckable(false);
	outputButton->setFixedSize(50, 50);
	outputButton->setStyleSheet(
	    "QPushButton {"
	    "  border-radius: 25px;"
	    "  background-color: red;"
	    "  color: white;"
	    "}");

	// Arrange input buttons in a grid layout
	QGridLayout* buttonLayout = new QGridLayout;
	buttonLayout->addWidget(physicalSwitch, 0, 0);
	buttonLayout->addWidget(webButton, 0, 1);
	buttonLayout->addWidget(timerForced, 1, 0);
	buttonLayout->addWidget(awayFromHome, 1, 1);
	buttonLayout->addWidget(timerGreen, 2, 0);
	buttonLayout->addWidget(excessGreen, 2, 1);

	// Create a group box for input buttons
	QGroupBox* inputGroupBox = new QGroupBox("Inputs");
	inputGroupBox->setLayout(buttonLayout);

	// Layout for the output button to center it
	QHBoxLayout* outputLayout = new QHBoxLayout;
	outputLayout->addStretch();
	outputLayout->addWidget(outputButton);
	outputLayout->addStretch();

	// Create the main layout
	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(clear);
	mainLayout->addWidget(debugTextArea);
	mainLayout->addWidget(inputGroupBox);
	mainLayout->addLayout(outputLayout); // Add the output button layout

	// Set the layout to the window
	window->setLayout(mainLayout);
	window->resize(500, 600); // Adjusted window size

	// Instantiate the Switch simulation object
	Switch* sw = new Switch;

	QObject::connect(clear, &QPushButton::clicked, [=]() {
		debugTextArea->clear();
	});

	// Redirect simulation output to the debug text area
	QObject::connect(sw, &Switch::debugMessage, [=](const QString& message) {
		debugTextArea->append(message);
		// Scroll to the bottom
		QScrollBar* bar = debugTextArea->verticalScrollBar();
		bar->setValue(bar->maximum());
	});

	// Connect input buttons to simulation inputs
	QObject::connect(physicalSwitch, &QPushButton::toggled, [=](bool checked) {
		changeButtonColor(physicalSwitch, checked);
		sw->inputs.physicalSwitch.set(checked);
		sw->multiStep();
	});

	QObject::connect(webButton, &QPushButton::toggled, [=](bool checked) {
		sw->inputs.webButton.toggle();
		sw->multiStep();
	});

	QObject::connect(timerForced, &QPushButton::toggled, [=](bool checked) {
		changeButtonColor(timerForced, checked);
		sw->inputs.timerForced.set(checked);
		sw->multiStep();
	});

	QObject::connect(awayFromHome, &QPushButton::toggled, [=](bool checked) {
		changeButtonColor(awayFromHome, checked);
		sw->inputs.awayFromHome.set(checked);
		sw->multiStep();
	});

	QObject::connect(timerGreen, &QPushButton::toggled, [=](bool checked) {
		changeButtonColor(timerGreen, checked);
		sw->inputs.timerGreen.set(checked);
		sw->multiStep();
	});

	QObject::connect(excessGreen, &QPushButton::toggled, [=](bool checked) {
		changeButtonColor(excessGreen, checked);
		sw->inputs.excessGreen.set(checked);
		sw->multiStep();
	});

	QObject::connect(sw, &Switch::stateChanged, [=](bool state) {
		changeButtonColor(webButton, state);
	});

	// Update the output lamp when the switch state changes
	QObject::connect(sw, &Switch::stateChanged, [=](bool state) {
		if (state) {
			outputButton->setText("On");
			outputButton->setStyleSheet(
			    "QPushButton {"
			    "  border-radius: 25px;"
			    "  background-color: green;"
			    "  color: white;"
			    "}");
		} else {
			outputButton->setText("Off");
			outputButton->setStyleSheet(
			    "QPushButton {"
			    "  border-radius: 25px;"
			    "  background-color: red;"
			    "  color: white;"
			    "}");
		}
	});

	// Show the window
	window->show();

	return app.exec();
}
