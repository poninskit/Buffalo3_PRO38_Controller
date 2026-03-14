// #include "StateManager.h"

// StateManager::StateManager() {
//     // Initialize with default values
//     currentState.input = OPT1;
//     currentState.volume = DEFAULT_VOL;
//     currentState.muted = false;
//     currentState.lockStatus = Unknown;
//     currentState.sampleRate = 0;
//     currentState.firShape = 0;
//     currentState.iirBandwidth = 0;
//     currentState.dpllBandwidth = 0;
//     currentState.jitterEliminator = 0;
// }

// void StateManager::updateInput(DAC_INPUT newInput) {
//     if (currentState.input != newInput) {
//         currentState.input = newInput;
//         if (stateChangeCallback) {
//             stateChangeCallback(currentState);
//         }
//     }
// }

// void StateManager::updateVolume(uint8_t newVolume, bool isMuted) {
//     bool changed = false;
//     if (currentState.volume != newVolume) {
//         currentState.volume = newVolume;
//         changed = true;
//     }
//     if (currentState.muted != isMuted) {
//         currentState.muted = isMuted;
//         changed = true;
//     }
//     if (changed && stateChangeCallback) {
//         stateChangeCallback(currentState);
//     }
// }

// void StateManager::updateLockStatus(LOCK_STATUS status) {
//     if (currentState.lockStatus != status) {
//         currentState.lockStatus = status;
//         if (stateChangeCallback) {
//             stateChangeCallback(currentState);
//         }
//     }
// }

// void StateManager::updateSampleRate(uint32_t rate) {
//     if (currentState.sampleRate != rate) {
//         currentState.sampleRate = rate;
//         if (stateChangeCallback) {
//             stateChangeCallback(currentState);
//         }
//     }
// }

// void StateManager::updateSettings(uint8_t fir, uint8_t iir, uint8_t dpll, uint8_t je) {
//     bool changed = false;
//     if (currentState.firShape != fir) {
//         currentState.firShape = fir;
//         changed = true;
//     }
//     if (currentState.iirBandwidth != iir) {
//         currentState.iirBandwidth = iir;
//         changed = true;
//     }
//     if (currentState.dpllBandwidth != dpll) {
//         currentState.dpllBandwidth = dpll;
//         changed = true;
//     }
//     if (currentState.jitterEliminator != je) {
//         currentState.jitterEliminator = je;
//         changed = true;
//     }
//     if (changed && stateChangeCallback) {
//         stateChangeCallback(currentState);
//     }
// }

// void StateManager::onStateChange(std::function<void(const DACState&)> callback) {
//     stateChangeCallback = callback;
// }