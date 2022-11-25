const check = document.querySelector('#CheckButton');
const fanon = document.querySelector('#FanButton');
const fanoff = document.querySelector('#FanButton2');
const windowopen = document.querySelector('#WindowButton');
const windowclose = document.querySelector('#WindowButton2');

const temp = document.querySelector('#Temp');
const humi = document.querySelector('#Humid');
const air = document.querySelector('#Airraid');

const API_URL = 'https://3m1hpgcf4m.execute-api.us-east-1.amazonaws.com/prod/devices/FinalExam';

const finddata = () => {
    fetch(API_URL)
        .then(response => response.json())
        .then(data => {
            let result = JSON.parse(data);
            document.getElementById('Temp').innerHTML = result.state.reported.temperature;
                document.getElementById('Humid').innerHTML = result.state.reported.humidity;
        });
}

const fan_on = () => {
    fetch(API_URL, {
        method: 'PUT',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
        })
    })
        .then(response => response.json())
        .then(data => {
            console.log(data);
        });
};

check.addEventListener('click', finddata);
