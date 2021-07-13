import { StatusBar } from 'expo-status-bar';
import React from 'react';
import HomePage from './pages/HomePage';

export default function App() {
  console.disableYellowBox = true;

  return (
    <HomePage/>
  );
}
