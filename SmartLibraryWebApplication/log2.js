const logbtn = document.querySelector('#LogCheckButton');
const log = document.querySelector('#logdata');
const from = document.querySelector('#from').value;
const to = document.querySelector('#to').value;

const API_URL = a;

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