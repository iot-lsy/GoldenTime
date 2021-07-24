package com.example.probonoapp;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class activity_menu extends AppCompatActivity {

    Button buttonLogout; //로그아웃버튼
    Button buttonIntroduce; //어플리케이션 소개 버튼
    Button buttonSettings; // 설정 버튼

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        Intent intent = new Intent(activity_menu.this, LoginActivity.class); //지금 액티비티에서 다른 액티비티로 이동하는 인텐트 설정
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);    //인텐트 플래그 설정
        startActivity(intent);  //인텐트 이동
        finish();   //현재 액티비티 종료
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_menu);

        buttonLogout = (Button)findViewById(R.id.buttonLogout);

        buttonLogout.setOnClickListener((new View.OnClickListener() { //로그아웃 버튼을 눌렀을 때
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(activity_menu.this,MainActivity.class); //화면 이동
                startActivity(intent);
                finish(); // 현재 액티비티 파괴
            }
        }));

        buttonIntroduce =(Button)findViewById(R.id.buttonIntroduce);
        buttonIntroduce.setOnClickListener((new View.OnClickListener() { //어플리케이션 소개 버튼을 눌렀을 때
            @Override
            public void onClick(View v) {
                Intent intent2 = new Intent(activity_menu.this, activity_introduce.class); //화면 이동
                startActivity(intent2);
                finish(); // 현재 액티비티 파괴
            }
        }));


    }
}