//******************************************************************************
// ES 9038 Pro, Version see definition
// Poninski Tomasz, 2023-01-20
// https://github.com/twistedpearaudio/Buffalo-III-SE-Pro-On-Board-Firmware
// https://twistedpearaudio.github.io/blog/docs_b3sepro/manual.html
//******************************************************************************

//Arduino enviroment
#include <Arduino.h>
//Header for global declarations
#include "globals.h"
//header for all DAC classes
#include "dac.h"
#include "Remote.h"
#include "Graphics.h"
#include "StateManager.h"

//------------------------------------------------------------------------------
#define READ_DAC_CYCLES   10000 //read the DAC Register every n-cycles, every 1s is enough
//------------------------------------------------------------------------------
// classes declarations

// DAC 
DAC* dac;
//REMOTE CONTROLL
RemoteInterface* remoteInterface;
//TOUCH INTERFACE
//lvgl handels touch already, so no need for a separate class, but keep reference to indev for polling
//SCREEN
Graphics* graphics;
//STATE MANAGER
StateManager* stateManager;



//------------------------------------------------------------------------------
//Variables
ACTION action = NONE;
ACTION lastAction = NONE;
int read_dac_counter = 0;
PAGE currentPage = MAIN_MENU;

//differentiate interfaces apply specific delay
INTERFACE interface = NONE_IFC;
//use delay if necessary to give user respond time
int delay_ms = 0;
//delay values
int delay_hold = 15;
int delay_switch = 400;
//dont draw sample rate if nothing changed
bool refreshSampleRate = true;
LOCK_STATUS last_lock = Unknown;
uint32_t last_fsr = 0;

void handleAction(ACTION action, int value = 0);

//******************************************************************************
// SETUP DAC
//******************************************************************************
void setup() {

    //Serial port for debugging
    if(DEBUG) Serial.begin(115200); // match monitor baud rate
    LOG("Debug mode\n");


    //initilize classes
    // Initialize hardware & interfaces


    // Graphics FIRST — it owns I2C for touch/display
    graphics        = new Graphics();
    stateManager    = new StateManager();
    remoteInterface = new RemoteInterface();

    // DAC AFTER graphics — uses separate I2C or same bus already initialized
    dac             = new DAC();
    dac->startDAC();

    // Both touch and remote go through the same function
    graphics->setActionCallback([](ACTION action, int value) {
        handleAction(action, value);
    });


    // Seed StateManager with current DAC values read from hardware
    if (dac->isAvailable()) {
        LOCK_STATUS lock_st = dac->getLockStatus();
        uint32_t    fsr     = dac->getRawSampleRate();

        stateManager->updateInput(dac->getInput());
        stateManager->updateVolume(dac->getVolume(), false);
        stateManager->updateLockStatus(
            lock_st, dac->dacLockString(lock_st)
        );
        stateManager->updateSampleRate(
            fsr, (lock_st != No_Lock) ? dac->getSampleRateString(fsr) : ""
        );
        stateManager->updateSettings(
            dac->getFIRShape(),
            dac->getIIRBandwidth(),
            dac->getDpllSerial(),
            dac->getJitterEl()
        );
    }
    // String conversion is pure lookup — no I2C, always safe to run
    {
        DACState s = stateManager->getState();
        stateManager->updateSettingsStrings(
            dac->getFIRShapeString(s.firShape),
            dac->getIIRBandwidthString(s.iirBandwidth),
            dac->getDpllSerialString(s.dpllBandwidth),
            dac->getJitterElString(s.jitterEliminator)
        );
    }


    // Register state change callback:
    // Whenever any state changes, push it to DAC hardware and redraw UI
    stateManager->onStateChange([](const DACState& s) {
        dac->setInput(s.input);
        dac->setVolume(s.muted ? MUTE_VOL : s.volume);
        

        graphics->printChannel(s.input);
        graphics->printVolume(s.muted ? MUTE_VOL : s.volume);
        graphics->printLockStatus(s.lockStatusStr);   // just a string
        graphics->printSampleRate(s.sampleRateStr);   // empty string = clears label
    });




    // Initial screen draw
    graphics->setDacAvailable(dac->isAvailable());
    graphics->printChannel(dac->getInput());
    graphics->printVolume(dac->getVolume());

}



//******************************************************************************
// LOOP THE DAC COMMANDS
//******************************************************************************
void loop() {

  
  
     // Remote input → same handler as touch
    ACTION action = remoteInterface->getAction(currentPage);
    
    if (action != NONE) {
        handleAction(action);
    } else {
        return;
    }

    // Periodic DAC polling
    if (read_dac_counter >= READ_DAC_CYCLES) {
        read_dac_counter = 0;

        bool wasAvailable = dac->isAvailable();
        bool nowAvailable = dac->checkAvailability();

        if (nowAvailable != wasAvailable) {
            graphics->setDacAvailable(nowAvailable);
        }

        if (nowAvailable) {
            LOCK_STATUS lock_st = dac->getLockStatus();
            uint32_t    fsr     = dac->getRawSampleRate();

            stateManager->updateLockStatus(
                lock_st,
                dac->dacLockString(lock_st)
            );
            stateManager->updateSampleRate(
                fsr,
                (lock_st != No_Lock) ? dac->getSampleRateString(fsr) : ""
            );
        }
    }


    read_dac_counter++;
  
}

//******************************************************************************
// HELPER FUNCTION TO HANDLE ACTIONS
//******************************************************************************
void handleAction(ACTION action, int value) {


    LOG("\nHandle action: " + String( action ));

    DACState s = stateManager->getState();

    switch (action) {
        case NONE:
            break;

        // direct input select from touch
        case CHANNEL_USB:
            dac->setInput(USB);
            stateManager->updateInput(USB);
            break;
        case CHANNEL_OPT1:
            dac->setInput(OPT1);
            stateManager->updateInput(OPT1);
            break;
        case CHANNEL_OPT2:
            dac->setInput(OPT2);
            stateManager->updateInput(OPT2);
            break;
        case CHANNEL_SPDIF:
            dac->setInput(SPDIF);
            stateManager->updateInput(SPDIF);
            break;
        // remote navigation — cycle through inputs
        case CHANNEL_LEFT:
            stateManager->updateInput(dac->decreaseInput());
            break;
        case CHANNEL_RIGHT:
            stateManager->updateInput(dac->increaseInput());
            break;


        case VOLUME_SET:
            dac->setVolume(value);
            stateManager->updateVolume(value, s.muted);
            break;
        case VOLUME_UP:
            dac->increaseVolume();
            if ( remoteInterface->isRepeat() ) dac->increaseVolume(); // extra step when held
            stateManager->updateVolume(dac->getVolume(), s.muted);
            break;
        case VOLUME_DOWN:
            dac->decreaseVolume();
            if ( remoteInterface->isRepeat() ) dac->decreaseVolume(); // extra step when held
            stateManager->updateVolume(dac->getVolume(), s.muted);
            break;


        case ENTER:
        case PLAY_PAUSE:
            stateManager->updateVolume(s.volume, !s.muted);
            break;

        case MENU:
            if (currentPage == MAIN_MENU) {
                currentPage = SETTINGS_MENU;
                graphics->showSettingsScreen();
                graphics->printSettings(stateManager->getState());
            } else {
                currentPage = MAIN_MENU;
                graphics->showMainScreen();
            }
            break;

        case SET_FIR_FILTER: {
            uint8_t val = dac->getCycleFIRShape();
            stateManager->updateSettings(val, s.iirBandwidth, s.dpllBandwidth, s.jitterEliminator);
            stateManager->updateSettingsStrings(
                dac->getFIRShapeString(val),
                dac->getIIRBandwidthString(s.iirBandwidth),
                dac->getDpllSerialString(s.dpllBandwidth),
                dac->getJitterElString(s.jitterEliminator)
            );
            graphics->printSettings(stateManager->getState(), 0);
            break;
        }
        case SET_IIR_BANDWIDTH: {
            uint8_t val = dac->getCycleIIRBandwidth();
            stateManager->updateSettings(s.firShape, val, s.dpllBandwidth, s.jitterEliminator);
            stateManager->updateSettingsStrings(
                dac->getFIRShapeString(s.firShape),
                dac->getIIRBandwidthString(val),
                dac->getDpllSerialString(s.dpllBandwidth),
                dac->getJitterElString(s.jitterEliminator)
            );
            graphics->printSettings(stateManager->getState(), 1);
            break;
        }
        case SET_DPLL: {
            uint8_t val = dac->getCycleDPLL();
            stateManager->updateSettings(s.firShape, s.iirBandwidth, val, s.jitterEliminator);
            stateManager->updateSettingsStrings(
                dac->getFIRShapeString(s.firShape),
                dac->getIIRBandwidthString(s.iirBandwidth),
                dac->getDpllSerialString(val),
                dac->getJitterElString(s.jitterEliminator)
            );
            graphics->printSettings(stateManager->getState(), 2);
            break;
        }
        case TOGGLE_JE: {
            uint8_t val = dac->getToggleJitterEliminator();
            stateManager->updateSettings(s.firShape, s.iirBandwidth, s.dpllBandwidth, val);
            stateManager->updateSettingsStrings(
                dac->getFIRShapeString(s.firShape),
                dac->getIIRBandwidthString(s.iirBandwidth),
                dac->getDpllSerialString(s.dpllBandwidth),
                dac->getJitterElString(val)
            );
            graphics->printSettings(stateManager->getState(), 3);
            break;
        }

        default:
            break;
    }
}


//******************************************************************************















