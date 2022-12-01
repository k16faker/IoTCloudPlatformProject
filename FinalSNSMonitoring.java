package com.example.lambda.monitoring;

import com.amazonaws.auth.AWSStaticCredentialsProvider;
import com.amazonaws.auth.BasicAWSCredentials;
import com.amazonaws.regions.Regions;
import com.amazonaws.services.lambda.runtime.Context;
import com.amazonaws.services.lambda.runtime.RequestHandler;
import com.amazonaws.services.sns.AmazonSNS;
import com.amazonaws.services.sns.AmazonSNSClientBuilder;
import com.amazonaws.services.sns.model.PublishRequest;
import com.amazonaws.services.sns.model.PublishResult;
import com.google.gson.JsonElement;
import com.google.gson.JsonParser;

public class FinalSNSMonitoring implements RequestHandler<Object, String> {
	int msg_count = 0; // 메일을 한번만 보내기 위해서 제어하기 위한 변수 선언

	@Override
	public String handleRequest(Object input, Context context) {
	    context.getLogger().log("Input: " + input);
	    String json = ""+input;
	    JsonParser parser = new JsonParser();
	    JsonElement element = parser.parse(json);
	    JsonElement state = element.getAsJsonObject().get("state");
	    JsonElement reported = state.getAsJsonObject().get("reported");
		
	    // 아두이노로부터 읽어온 JSON 문서에서 온습도값, Co2 값, 현재 인원수, 팬의 작동상태만을 가져옴
	    String temperature = reported.getAsJsonObject().get("temperature").getAsString();
	    String humidity = reported.getAsJsonObject().get("humidity").getAsString();
	    String co2 = reported.getAsJsonObject().get("CO2").getAsString();
	    String people = reported.getAsJsonObject().get("R").getAsString();
	    String fan = reported.getAsJsonObject().get("FAN").getAsString();
	    
	    // String에서 각각에 맞는 형식으로 변환
	    double temp = Double.valueOf(temperature);
	    double humi = Double.valueOf(humidity);
	    int c = Integer.valueOf(co2);
	    int p = Integer.valueOf(people);
	    String f = String.valueOf(fan);
	    
	    final String AccessKey="Privacy AccessKey";
	    final String SecretKey="Privacy SecretKey";
	    final String topicArn="Privacy TopicKey";

	    BasicAWSCredentials awsCreds = new BasicAWSCredentials(AccessKey, SecretKey);  
	    AmazonSNS sns = AmazonSNSClientBuilder.standard()
	                .withRegion(Regions.AP_NORTHEAST_2)
	                .withCredentials( new AWSStaticCredentialsProvider(awsCreds) )
	                .build();
		
	    // 보낼 메일의 내용과 제목을 설정
	    final String msg = "현재 열람실의 인원은 총 " + p + "명이므로 빈 좌석이 없습니다. 찾아주셔서 감사합니다.";
	    final String subject = "현재 열람실이 만석입니다.";
		
	    // RFID가 한 개밖에 없으므로, RFID가 한번 찍히면(만석이면) 이메일 전송
	    if (p >= 1) {
	    	if (msg_count==0) { // 메일을 계속해서 보낼 수 없으니까 한번만 보내기 위해서  다음과 같은 조건을 걸음
	        PublishRequest publishRequest = new PublishRequest(topicArn, msg, subject);
	        PublishResult publishResponse = sns.publish(publishRequest);
	        msg_count++;
	    	}
	    }

	    return "전송 완료";
	}
}
