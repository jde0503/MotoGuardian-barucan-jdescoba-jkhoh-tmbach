# ---- Import necessary packages and modules ----
from flask import Flask, render_template

# ---- APP SETUP ----
app = Flask(__name__)


# ---- DB SETUP ----


# ---- ROUTES ----

# ---------------------------------Home---------------------------------
@app.route('/landing')
def land():
    return render_template('landing.html')
