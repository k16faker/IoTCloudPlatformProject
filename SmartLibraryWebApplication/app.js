const check = document.querySelector('#CheckButton');
const log = document.querySelector('#LogButton');
const log2 = document.querySelector('#LogButton2');
const fanon = document.querySelector('#FanButton');
const fanoff = document.querySelector('#FanButton2');
const windowopen = document.querySelector('#WindowButton');
const windowclose = document.querySelector('#WindowButton2');

const temp = document.querySelector('#Temp');
const humi = document.querySelector('#Humid');
const air = document.querySelector('#Airraid');

//온습도 API
const API_URL = 'https://3m1hpgcf4m.execute-api.us-east-1.amazonaws.com/prod/devices/FinalExam';
//대기질 API
const API_URL2 = 'https://vilk05yqef.execute-api.ap-northeast-2.amazonaws.com/prod/finalexam';


//API 호출 및 데이터 출력 함수
const finddata = () => {
    fetch(API_URL)
        .then(response => response.json())
        .then(data => {
            console.log(data);
            let result = JSON.parse(data);
            document.getElementById('Temp').innerHTML = result.state.reported.temperature;
            document.getElementById('Humid').innerHTML = result.state.reported.humidity;
        })
        .catch((error) => {
            alert('error');

        });
        fetch(API_URL2)
        .then(response => response.json())
        .then(data => {
            console.log(data);
            let result = JSON.parse(data);
            document.getElementById('Airraid').innerHTML = "";
        })
        .catch((error) => {
            alert('error');
        });
}

//3초에 1회씩 자동 갱신되도록 하는 함수
const find_find = () => {
    setInterval(finddata, 3000);
}

//온습도 로그 페이지 띄우기
const movelink = () => {
    window.open("index_log.html");
}
//대기질 로그 페이지 띄우기
const movelink2 = () => {
    window.open("index_log2.html");
}

//팬 켜기 함수
const fan_on = () => {
    fetch(API_URL, {
        method: 'PUT',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            "LED": "ON"
        })
    })
        .then(response => response.json())
        .then(data => {
            console.log(data);
        });
};

//팬 끄는 함수
const fan_off = () => {
    fetch(API_URL, {
        method: 'PUT',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(
        {
            "tags" : [
                {
                    "tagName" : "LED",
                    "tagValue" : "OFF"
                }
            ]
        })
    })
        .then(response => response.json())
        .then(data => {
            console.log(data);
        });
};

//이벤트 리스너들
check.addEventListener('click', find_find);
log.addEventListener('click', movelink);
log2.addEventListener('click', movelink2);