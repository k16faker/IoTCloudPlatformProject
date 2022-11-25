const check = document.querySelector('#CheckButton');
const fanon = document.querySelector('#FanButton');
const fanoff = document.querySelector('#FanButton2');
const windowopen = document.querySelector('#WindowButton');
const windowclose = document.querySelector('#WindowButton2');

const temp = document.querySelector('#Temp');
const humi = document.querySelector('#Humid');
const air = document.querySelector('#Airraid');

const API_URL = '';

const finddata = () => {
    fetch(API_URL)
        .then(response => response.json())
        .then(data => {
            let result = JSON.parse(data);
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
