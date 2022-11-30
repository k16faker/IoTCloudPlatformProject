package com.example.lambda.recording;

import java.text.SimpleDateFormat;
import java.util.TimeZone;

import com.amazonaws.services.dynamodbv2.AmazonDynamoDB;
import com.amazonaws.services.dynamodbv2.AmazonDynamoDBClientBuilder;
import com.amazonaws.services.dynamodbv2.document.DynamoDB;
import com.amazonaws.services.dynamodbv2.document.Item;
import com.amazonaws.services.dynamodbv2.document.PutItemOutcome;
import com.amazonaws.services.dynamodbv2.document.spec.PutItemSpec;
import com.amazonaws.services.dynamodbv2.model.ConditionalCheckFailedException;
import com.amazonaws.services.lambda.runtime.Context;
import com.amazonaws.services.lambda.runtime.RequestHandler;

public class FinalRecordingDeviceInfoHandler implements RequestHandler<Thing, String> {
    private DynamoDB dynamoDb;
    private String DYNAMODB_TABLE_NAME = "FinalDeviceData"; //새로운 테이블로 변경해줌

    @Override
    public String handleRequest(Thing input, Context context) {
        this.initDynamoDbClient();

        persistData(input);
        return "Success in storing to DB!";
    }

    private PutItemOutcome persistData(Thing thing) throws ConditionalCheckFailedException {

        // Epoch Conversion Code: https://www.epochconverter.com/
        SimpleDateFormat sdf = new SimpleDateFormat ("yyyy-MM-dd HH:mm:ss");
        sdf.setTimeZone(TimeZone.getTimeZone("Asia/Seoul"));
        String timeString = sdf.format(new java.util.Date (thing.timestamp*1000));

        return this.dynamoDb.getTable(DYNAMODB_TABLE_NAME) // 여기도 받는 값을 수정
                .putItem(new PutItemSpec().withItem(new Item().withPrimaryKey("time", thing.timestamp)
                        .withString("Temperature", thing.state.reported.temperature)
                        .withString("Humidity", thing.state.reported.humidity)
                        .withString("Co2", thing.state.reported.CO2)
                        .withString("PEOPLE", thing.state.reported.R)
                        .withString("FAN", thing.state.reported.FAN)
                        .withString("Timestamp",timeString)));
    }

    private void initDynamoDbClient() {
        AmazonDynamoDB client = AmazonDynamoDBClientBuilder.standard().withRegion("ap-northeast-2").build();

        this.dynamoDb = new DynamoDB(client);
    }

}

class Thing {
    public State state = new State();
    public long timestamp;

    public class State {
        public Tag reported = new Tag();
        public Tag desired = new Tag();

        public class Tag { // 받는 값을 수정
            public String temperature;
            public String humidity;
            public String CO2;
            public String R;
            public String FAN;
        }
    }
}