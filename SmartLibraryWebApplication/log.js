const logbtn = document.querySelector('#LogCheckButton');
const log = document.querySelector('#logdata');
const from = document.getElementById('from').value;
const to = document.getElementById('to').value;

let API_URL = "https://3m1hpgcf4m.execute-api.us-east-1.amazonaws.com/prod/devices/TempAndHum/log?from="+ from +"%2000:00:00&to="+ to+"%2018:09:36";

const finddata = () => {
    fetch(API_URL)
        .then(response => response.json())
        .then(data => {
            console.log(data);
            let result = JSON.parse(JSON.stringify(data));
            document.getElementById('logdata').innerHTML = result;
        })
        .catch((error) => {
            console.log('error');
        });
}

logbtn.addEventListener('click', finddata);