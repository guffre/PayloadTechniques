from flask import Flask
app = Flask(__name__)

@app.route("/")
def hello():
    return "Hello World!"

@app.route("/node/<name>")
def node(name):
    return("exec dir",200)

if __name__ == "__main__":
    app.run(port=80)
