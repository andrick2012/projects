from flask import Flask, render_template, request, redirect,url_for
import pprint
import getpass
import ccqclient
from ccqclient import CCQCloud, CCQScheduler, CCQJob
import pprint
import urllib
import json
login_instance_url='login1-spinelcichlidsnake.cloudycluster.net'
user_name = 'boyd'
user_pass = '0mnipWns'
client = ccqclient.CCQClient(login_instance_url, user_name, user_pass, CCQCloud.GCP, CCQScheduler.Slurm)
def ccqdel(hostname, username, password, jobId):
    encodedUserName = username
    encodedPassword = password
    jobForceDelete = ""
    valKey = "unpw"
    dateExpires = ""
    certLength = 0
    ccAccessKey = ""
    remoteUserName = username
    databaseDelete = ""
    data = {"jobId": str(jobId), "userName": str(encodedUserName), "instanceId": None, "jobNameInScheduler": None, "password": str(encodedPassword), "jobForceDelete": jobForceDelete, 'schedulerType': None, 'schedulerInstanceId': None, 'schedulerInstanceName': None, 'schedulerInstanceIp': None, "valKey": str(valKey), "dateExpires": str(dateExpires), "certLength": str(certLength), "ccAccessKey": str(ccAccessKey), "remoteUserName": str(remoteUserName), "databaseDelete": str(databaseDelete)}
    url = "https://%s/srv/ccqdel" % hostname
    data = json.dumps(data).encode()
    headers = {"Content-Type": "application/json"}
    request = urllib.request.Request(url, data, headers)
    response = urllib.request.urlopen(request).read().decode()
    response = json.loads(response)
    return response["payload"]["message"]
app = Flask(__name__)

@app.route('/')
def index():
  return render_template('index.html', host=login_instance_url , passCode=user_pass, uName=user_name)

@app.route('/submit')
def submit():
  return render_template('submit.html')

@app.route('/subJub', methods=['POST'])
def subJub():
  num_nodes = int(request.form['numNodes'])
  numTasksPerNode = int(request.form['numTasksPerNode'])
  filePath = str(request.form['filePath'])
  fileName = str(request.form['fileName'])
  exeDropDown = str(request.form['exeDropDown'])
  runDropDown = str(request.form['runDropDown'])
  the_script = '#!/bin/bash \\n#SBATCH -N '+str(num_nodes)+'\\n#SBATCH --ntasks-per-node'+str(numTasksPerNode)+'\\n export SHARED_FS_NAME='+filePath+'\\nmodule add openmpi\\3.0.0 \\ncd $SHARED_FS_NAME \\n'+exeDropDown+' '+fileName+'.c -o '+fileName+'-lm \\n'+runDropDown+'-np '+str(num_nodes*numTasksPerNode)+'$SHARED_FS_NAME\\'+fileName+'\n'        
  output = client.ccqsub(filePath+'/',fileName,job_body=the_script)
  return redirect(url_for('home',stateUp=output))

@app.route('/home',methods = ['POST', 'GET'])
def home():
	if request.method == 'POST':
		global login_instance_url
		global user_name
		global user_pass
		global client
		login_instance_url=request.form['hname']
		user_name=request.form['uname']
		user_pass=request.form['psw']
		client = ccqclient.CCQClient(login_instance_url, user_name, user_pass, CCQCloud.GCP, CCQScheduler.Slurm)
	return render_template('home.html',stateUp='')

@app.route('/status',methods = ['POST', 'GET'])
def status():
   	try:
   		output = client.ccqstat()
   		status_list=[]
   		for job in output:
   			status_list.append((str(job.job_id),str(job.job_name),str(job.scheduler_name),str(job.job_status)))
   		return render_template('status.html', result=status_list)
   	except urllib.request.URLError as err:
   		print("URL error: {0}".format(err))

@app.route('/postmethod', methods = ['POST'])
def postmethod():
  job_id=request.form['myData']
  print(job_id)
  ccqdel(login_instance_url, user_name, user_pass, job_id)
  return render_template('home.html')
    
if __name__ == '__main__':
   app.run(debug = True)