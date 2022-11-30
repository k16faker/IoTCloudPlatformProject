const logbtn = document.querySelector('#LogCheckButton');
const log = document.querySelector('#logdata');
const from = document.getElementById('from');
const to = document.getElementById('to');

let API_URL = "https://eqbyoluqw4.execute-api.ap-northeast-2.amazonaws.com/prod/devices/FinalDeviceData/log?from="+from.value+"%2000:00:00&to="+to.value+"%2000:00:36";


const finddata = () => {
    fetch("https://eqbyoluqw4.execute-api.ap-northeast-2.amazonaws.com/prod/devices/FinalDeviceData/log?from="+from.value+"%2000:00:00&to="+to.value+"%2000:00:36")
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

//let result = JSON.parse(JSON.stringify(data));
//document.getElementById('logdata').innerHTML = result;