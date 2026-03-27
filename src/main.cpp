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
#include "driver/i2c.h"

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
static uint32_t lastPoll = 0;
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


void scanAndPrintI2C(){
    Serial.println("I2C scan on bus 0 (GPIO 19=SDA, 20=SCL):");
    for (uint8_t addr = 0x03; addr <= 0x77; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t res = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(10));
        i2c_cmd_link_delete(cmd);
        if (res == ESP_OK) {
            Serial.printf("  Found: 0x%02X\n", addr);
        }
    }
    Serial.printf("Scan done \n");
}

//******************************************************************************
// SETUP DAC
//******************************************************************************
void setup() {

    //Serial port for debugging
    if(DEBUG) Serial.begin(115200); // match monitor baud rate
    LOG("Debug mode\n");


    // Initialize hardware & interfaces
    // Graphics FIRST — it owns I2C for touch/display
    graphics        = new Graphics(); //Touch GT911 Initilizes I2C, so do it before DAC which also uses I2C, but can share the bus just fine. Also we want to show DAC status on boot, so graphics needs to be ready first.
    remoteInterface = new RemoteInterface();
    stateManager    = new StateManager();
    stateManager->loadState();
    // DAC AFTER graphics — uses separate I2C or same bus already initialized
    dac             = new DAC();
    
    
    // apply initial settings from state manager to DAC hardware
    DACState s = stateManager->getState();
    dac->setInput(s.input);
    dac->setVolume(s.muted ? MUTE_VOL : s.volume);
    dac->setFIRShape(s.firShape);
    dac->setIIRBandwidth(s.iirBandwidth);
    dac->setDpllSerial(s.dpllBandwidth);
    dac->setJitterEl(s.jitterEliminator);


    // // Seed StateManager with current DAC values read from hardware
    // if (dac->isAvailable()) {
    //     LOCK_STATUS lock_st = dac->getLockStatus();
    //     uint32_t    fsr     = dac->getRawSampleRate();

    //     stateManager->updateInput(dac->getInput());
    //     stateManager->updateVolume(dac->getVolume(), false);
    //     stateManager->updateLockStatus(
    //         lock_st, dac->dacLockString(lock_st)
    //     );
    //     stateManager->updateSampleRate(
    //         fsr, (lock_st != No_Lock) ? dac->getSampleRateString(fsr) : ""
    //     );
    //     stateManager->updateSettings(
    //         dac->getFIRShape(),
    //         dac->getIIRBandwidth(),
    //         dac->getDpllSerial(),
    //         dac->getJitterEl()
    //     );
    // }


    // String conversion is pure lookup — no I2C, always safe to run
    stateManager->updateSettingsStrings(
        dac->getFIRShapeString(s.firShape),
        dac->getIIRBandwidthString(s.iirBandwidth),
        dac->getDpllSerialString(s.dpllBandwidth),
        dac->getJitterElString(s.jitterEliminator)
    );
    

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


    // Grahics and Remote action callback:
    // Both touch and remote go through the same function
    graphics->setActionCallback([](ACTION action, int value) {
        handleAction(action, value);
    });


    // Initial screen draw
    // sample rate and lock status will be updated in the first loop after DAC polling, so no need to set here
    graphics->printChannel(s.input);
    graphics->printVolume(s.muted ? MUTE_VOL : s.volume);

}



//******************************************************************************
// LOOP THE DAC COMMANDS
//******************************************************************************
void loop() {


     // Remote input → same handler as touch
    ACTION action = remoteInterface->getAction(currentPage);
    if (action != NONE) handleAction(action);


    // Periodic DAC polling
    if (millis() - lastPoll >= 1000) {  // every 1 second
        lastPoll = millis();

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
            if (remoteInterface->isRepeat()) { dac->increaseVolume(); }//double the speed
            stateManager->updateVolume(dac->increaseVolume(), s.muted);
            break;
        case VOLUME_DOWN:
            if (remoteInterface->isRepeat()) { dac->decreaseVolume(); }//double the speed
            
            stateManager->updateVolume(dac->decreaseVolume(), s.muted);
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
            uint8_t val = dac->cycleFIRShape();
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
            uint8_t val = dac->cycleIIRBandwidth();
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
            uint8_t val = dac->cycleDPLL();
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
            uint8_t val = dac->toggleJitterEliminator();
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















