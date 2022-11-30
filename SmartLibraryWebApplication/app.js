const check = document.querySelector('#CheckButton');
const log = document.querySelector('#LogButton');
const fanon = document.querySelector('#FanButton');
const fanoff = document.querySelector('#FanButton2');
const windowopen = document.querySelector('#WindowButton');
const windowclose = document.querySelector('#WindowButton2');

const temp = document.querySelector('#Temp');
const humi = document.querySelector('#Humid');
const air = document.querySelector('#Airraid');
const fans = document.querySelector('#fans');
const max = document.querySelector('#max');

//온습도 API
const API_URL = 'https://eqbyoluqw4.execute-api.ap-northeast-2.amazonaws.com/prod/devices/finalexam';
//상태변경 API


//API 호출 및 데이터 출력 함수
const finddata = () => {
    fetch(API_URL)
        .then(response => response.json())
        .then(data => {
            console.log(data);
            let result = JSON.parse(data);
            document.getElementById('Temp').innerHTML = result.state.reported.temperature;
            document.getElementById('Humid').innerHTML = result.state.reported.humidity;
            document.getElementById('Airraid').innerHTML = result.state.reported.CO2;
            document.getElementById('stds').innerHTML = result.state.reported.R;
            document.getElementById('fans').innerHTML = result.state.reported.FAN;
        })
        .catch((error) => {
            alert('error');

        });
}


//3초에 1회씩 자동 갱신되도록 하는 함수
const find_find = () => {
    finddata();
    setInterval(finddata, 3000);
}

//로그 페이지 띄우기
const movelink = () => {
    window.open("index_log.html");
}

//팬 켜기 함수
const fan_on = () => {
    fetch("https://eqbyoluqw4.execute-api.ap-northeast-2.amazonaws.com/prod/devices/finalexam", {
        method: "PUT",
        headers: {
            'Content-Type': 'application/json'
        },
        body:JSON.stringify({ tags : [ { tagName : "FAN", tagValue : "ON" } ] })
    })
        .then(response => response.json())
        .then(data => {
            console.log(data);
        });
};

//팬 끄는 함수
const fan_off = () => {
    fetch("https://eqbyoluqw4.execute-api.ap-northeast-2.amazonaws.com/prod/devices/finalexam", {
        method: 'PUT',
        headers: {
            'Content-Type':'application/json'
        },
        body:JSON.stringify({ tags : [ { tagName : "FAN", tagValue : "OFF" } ] })
    })
        .then(response => response.json())
        .then(data => {
            console.log(data);
        });
};

//이벤트 리스너들
check.addEventListener('click', find_find);
log.addEventListener('click', movelink);
fanon.addEventListener('click', fan_on);
fanoff.addEventListener('click', fan_off);


