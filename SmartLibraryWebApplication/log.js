const logbtn = document.querySelector('#LogCheckButton');
const log = document.querySelector('#logdata');
const from = document.querySelector('#from').value;
const to = document.querySelector('#to').value;

const API_URL = 'https://3m1hpgcf4m.execute-api.us-east-1.amazonaws.com/prod/devices/TempAndHum/log?from=2022-11-25%2000:00:00&to=2022-11-27%2018:09:36';


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