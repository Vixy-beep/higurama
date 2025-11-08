from flask import Flask, request, render_template, redirect, url_for
import os

app = Flask(__name__)

# Ruta para enviar comandos al malware
@app.route('/command', methods=['POST'])
def command():
    target = request.form['target']
    cmd = request.form['cmd']
    # Aquí podrías tener lógica para enviar el comando a la máquina específica
    # Por ejemplo, podrías tener un diccionario de máquinas infectadas y sus IDs
    # y enviar el comando a la máquina correspondiente
    return redirect(url_for('index'))

# Ruta para la interfaz web
@app.route('/')
def index():
    return render_template('index.html')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
