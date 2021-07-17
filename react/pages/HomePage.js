import React from 'react';
import { StyleSheet, Text, View, Image, TouchableOpacity, ScrollView, StatusBar} from 'react-native';
import safe from '../assets/safe.png';

export default function HomePage(){
    console.disableYellowBox = true;

    return (
        <View style={styles.container}>
             <StatusBar style="light" />
        
            <View style={styles.textContainer}>
                <Text style={styles.title}>안전바 응급호출 시스템</Text>
                <Image style={styles.safeImage} source={safe} resizeMode={"cover"}/>
                <TouchableOpacity style={styles.button}>
                    <Text style={styles.buttonText}>  로그인  </Text>
                </TouchableOpacity>
                <TouchableOpacity style={styles.button02}>
                    <Text style={styles.buttonText}>회원가입</Text>
                </TouchableOpacity>
            </View>
        </View>)
}

const styles = StyleSheet.create({
    container: {
        flex:1,
        backgroundColor:"#1F266A",
        alignItems:"center"
      
    },
    title: {
        fontSize:25,
        fontWeight:"700",
        color:"#000",
        paddingLeft:30,
        paddingRight:30,
    },
    textContainer: {
        width:300,
        height:570,
        backgroundColor:"#fff",
        marginTop:50,
        borderRadius:30,
        justifyContent:"center",
        alignItems:"center"
    },
    safeImage:{
        width:200,
        height:200,
        borderRadius:30,
        marginTop:30,
    },
    button:{
        backgroundColor:"#FBD652",
        padding:20,
        borderRadius:15,
        marginTop:40,
        marginBottom:20,
    },
    button02:{
        backgroundColor:"#FBD652",
        padding:20,
        borderRadius:15
    },
    buttonText: {
        color:"#fff",
        fontSize:20,
        fontWeight:"700"
    }
})