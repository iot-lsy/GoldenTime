package com.example.probonoapp;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.auth.AuthResult;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;

public class activity_register extends AppCompatActivity {


    private FirebaseAuth mFirebaseAuth; //파이어베이스 인증처리
    private DatabaseReference mDatabaseRef; // 실시간 데이터베이스 (서버연동)
    private EditText mEtName, EmtPwd,mEtPwd2, mEtGender, mEtLocate, mETPhonenumber, mEtBirth;  // 회원가입 입력필드
    private Button mBtnRegister;  //회원가입 버튼

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        Intent intent = new Intent(activity_register.this, LoginActivity.class); //지금 액티비티에서 다른 액티비티로 이동하는 인텐트 설정
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);    //인텐트 플래그 설정
        startActivity(intent);  //인텐트 이동
        finish();   //현재 액티비티 종료
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_register);

        mFirebaseAuth = FirebaseAuth.getInstance();
        mDatabaseRef = FirebaseDatabase.getInstance().getReference("ProbonoApp");

        mEtName = findViewById(R.id.et_email);
        EmtPwd = findViewById(R.id.et_pwd);
        mEtPwd2 = findViewById(R.id.et_pwd2);
        mEtGender = findViewById(R.id.et_gender);
        mEtLocate = findViewById(R.id.et_locate);
        mETPhonenumber = findViewById(R.id.et_phonenumber);
        mEtBirth = findViewById(R.id.et_birth);


        mBtnRegister = findViewById(R.id.btn_register);



        mBtnRegister.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // 회원가입 버튼 누를시
                String strEmail = mEtName.getText().toString();
                String strPwd = EmtPwd.getText().toString();
                String strPwd2 = mEtPwd2.getText().toString();
                String strGender = mEtGender.getText().toString();
                String strLocate = mEtLocate.getText().toString();
                String strPhonenumber = mETPhonenumber.getText().toString();
                String strBirth = mEtBirth.getText().toString();

                if (mEtName.getText().toString().length()==0){
                    Toast.makeText(getApplicationContext(),"아이디를 입력하세요!",Toast.LENGTH_SHORT).show();
                    mEtName.requestFocus();
                    return;
                }
                if (mEtGender.getText().toString().length()==0){
                    Toast.makeText(getApplicationContext(),"성별을 입력하세요!",Toast.LENGTH_SHORT).show();
                    mEtName.requestFocus();
                    return;
                }
                if (mEtLocate.getText().toString().length()==0){
                    Toast.makeText(getApplicationContext(),"주소를 입력하세요!",Toast.LENGTH_SHORT).show();
                    mEtName.requestFocus();
                    return;
                }
                if (mETPhonenumber.getText().toString().length()==0){
                    Toast.makeText(getApplicationContext(),"전화번호를 입력하세요!",Toast.LENGTH_SHORT).show();
                    mEtName.requestFocus();
                    return;
                }
                if(!EmtPwd.getText().toString().equals(mEtPwd2.getText().toString())){
                    Toast.makeText(getApplicationContext(),"비밀번호가 일치하지 않습니다!",Toast.LENGTH_SHORT).show();
                    EmtPwd.setText("");
                    mEtPwd2.setText("");
                    EmtPwd.requestFocus();
                    return;
                }

                //firebase Auth 진행
                mFirebaseAuth.createUserWithEmailAndPassword(strEmail,strPwd).addOnCompleteListener(activity_register.this, new OnCompleteListener<AuthResult>() {
                    @Override
                    public void onComplete(@NonNull Task<AuthResult> task) {
                        if (task.isSuccessful()){
                            FirebaseUser firebaseUser = mFirebaseAuth.getCurrentUser();
                            UserAccount account = new UserAccount();
                            account.setIdToken(firebaseUser.getUid());
                            account.setEmailID(firebaseUser.getEmail());
                            account.setPassword(strPwd);
                            account.setGender(strGender);
                            account.setBirth(strBirth);
                            account.setLocate(strLocate);
                            account.setPhoneNumber(strPhonenumber);

                            
                            //setValue : 데이터베이스에 삽입
                            mDatabaseRef.child("UserAccount").child(firebaseUser.getUid()).setValue(account);

                            Toast.makeText(activity_register.this,"회원가입 성공!",Toast.LENGTH_SHORT).show();
                        }
                        else{
                            Toast.makeText(activity_register.this,"회원가입 실패!",Toast.LENGTH_SHORT).show();
                        }
                    }
                });
            }
        });
    }


}