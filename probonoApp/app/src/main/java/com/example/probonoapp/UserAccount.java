package com.example.probonoapp;

//사용자 계정 정보 모델 클래스

public class UserAccount {

    private String idToken;   //Firebase Uid (고유 토큰 정보)
    private String emailID;   //이메일 아이디
    private String password;  //비밀번호
    private String gender; //성별
    private String birth; //생년월일
    private String phoneNumber; //전화번호
    private String locate; //주소

    public UserAccount(){ }

    public String getIdToken() {return idToken;}

    public void setIdToken(String idToken) {this.idToken = idToken;}

    public String getEmailID(){return emailID;}

    public void setEmailID(String emailID){
        this.emailID = emailID;
    }

    public String getPassword(){
        return password;
    }

    public void setPassword(String password){
        this.password = password;
    }

    public String gender(){ return gender;}

    public void setGender(String strGender){ this.gender = gender;}

    public String birth(){ return birth;}

    public void setBirth(String strBirth){ this.birth = birth;}

    public String getPhoneNumber(){ return phoneNumber;}

    public void setPhoneNumber(String strPhonenumber){ this.phoneNumber = phoneNumber;}

    public String locate(){ return locate;}

    public void setLocate(String strLocate){ this.locate = locate;}


}
