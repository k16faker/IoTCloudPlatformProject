# IoTCloudPlatformProject

SmartLibraryWebApplication
<p>데이터 확인 및 통제가 가능한 웹 어플리케이션</p>




어플리케이션의 모습(SmartLibraryWebApplication 폴더의 index.html 파일을 CORS 제한이 풀린 크롬으로 실행시켜야함)
![웹사진](https://user-images.githubusercontent.com/93112224/204965968-394a78a2-3a99-4f7b-aaa0-aa8e79400826.jpg)

버튼 설명

1.데이터 조회
아두이노로부터 받은 데이터(온도, 습도, 대기질, 현재 인원 수, Fan의 상태)를 바로 밑의 각 칸에 표시한다.

2.로그 조회
로그 조회 페이지로 이동

3.Fan On/Off 버튼
아두이노에 장착되어있는 Fan 모듈을 On/Off한다.



<h3>사용법 순서</h3>

1. AWS_IoT.ino 아두이노 파일과 Lambda함수, AWS를 이용해 API를 구축한다.

2. 구축된 API 링크중 상태 조회 링크는 app.js의 API_URL에, 로그 조회 API링크는 log.js의 API_URL에 넣는다.

3.SmartLibraryWebApplication 폴더의 index.html 파일을 실행



<h3>로그 조회 페이지 사용법</h3>

![KakaoTalk_20221202_002224202](https://user-images.githubusercontent.com/93112224/205477821-0fbe77ea-88bc-4e67-bf66-819eeae82688.jpg)

상단 빈 칸 두곳에 원하는 날짜 범위를 yyyy-mm-dd 형식으로 작성한 후 "로그 확인하기" 버튼 클릭

