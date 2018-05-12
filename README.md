# MotoGuardian
Project for ECE 140B - Spring 2018

### Members:
* Joshua Escobar      | A11606542
* Bronson Arucan      | A11291138
* Janet Hoh           | A11441575
* Treit Bach          | A11860217

### Info:
This repo contains folders for each assignment given in ECE 140B. Within these folders, there are other repos (submodules) for the motoguarian web app. These submodules are the repos that are pushed to heroku.

Public Link: http://motoguardian.herokuapp.com

### How to Install:
1. Open Terminal
2. 'cd' to whatever folder you want to put this repository in. (Ex: 'cd ~/Projects')
3. 'git clone https://github.com/jde0503/MotoGuardian-barucan-jdescoba-jkhoh-tmbach.git'
4. Login to GitHub account if necessary

### Installing and Updating Submodules:
1. 'git submodule init' to initialize any submodules in the repository
2. 'git submodule update' to update any submodules
3. To run both in just one command, use 'git submodule update --init'

### Installing a Virtual Environment (Optional)
Installing a virtual environment allows you to manage different Python environments and is designed to prevent conflicts between projects.

1. Run 'pip install virtualenv' to install virtualenv on your system
2. Run 'virtualenv --version' to check if virtualenv was installed successfully. It should read '15.2.0'
3. Ensure that you're currently in the Motoguardian directory. It should read something like '~/Projects/Motoguardian/'
4. 'cd' into the directory in which you want to create a virtual environment
5. Run 'virtualenv venv' to set up a virtual environment. Check if a folder named 'venv' was created in your directory
6. 'source venv/bin/activiate' to activate the virtual environment.
   * This will activate the virtual environment. Note that your terminal will say something like '(venv) user$'
7. Leave the enviroment using 'deactivate'

### Installing the Project
1. Run 'pip install -r requirements.txt' to install all packages specified in 'requirements.txt'
2. Setup the database by running './manage.py migrate'

### Running the Django Project
1. 'cd' into the Motoguardian directory if you are not already there.
2. Run 'python manage.py runserver'
3. Visit 'http://127.0.0.1:8000' in your browser.
   * The application should load
