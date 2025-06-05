import flask
import threading
import time
import re
import os

app = flask.Flask(__name__)
last_command = {"command": "", "result": "", "success": True}

# Ustaw ścieżkę do pliku latest.log w tym samym folderze co skrypt
LOG_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "latest.log")

def follow_log():
    global last_command
    with open(LOG_PATH, "r", encoding="utf-8") as f:
        f.seek(0, 2)
        while True:
            line = f.readline()
            if not line:
                time.sleep(0.1)
                continue
            # Przykład: [12:34:56] [Server thread/INFO]: unitr issued server command: /give unitr diamond 1
            m = re.search(r'(\w+) issued server command: (/.+)', line)
            if m:
                last_command["command"] = m.group(2)
                last_command["result"] = ""
                last_command["success"] = True
            # Przykład wyniku: [12:34:56] [Server thread/INFO]: Given [Diamond] * 1 to unitr
            elif last_command["command"]:
                # Możesz dodać tu własne reguły rozpoznawania sukcesu/błędu
                if "Given" in line:
                    last_command["result"] = line.strip().split("INFO]:")[-1].strip()
                    last_command["success"] = True
                elif "error" in line.lower() or "not found" in line.lower():
                    last_command["result"] = line.strip().split("INFO]:")[-1].strip()
                    last_command["success"] = False

@app.route("/last_command")
def get_last_command():
    return flask.jsonify(last_command)

@app.route("/")
def index():
    return "<h2>Serwer działa! Użyj <a href='/last_command'>/last_command</a> aby pobrać ostatnią komendę.</h2>"

if __name__ == "__main__":
    threading.Thread(target=follow_log, daemon=True).start()
    app.run(host="0.0.0.0", port=8080)
