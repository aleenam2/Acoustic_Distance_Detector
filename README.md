# Acoustic_Distance_Detector
## Description

The goal of the project is to develop a user-friendly smartphone application that can accurately map the walls in indoor spaces. The basic algorithm we will use in this project is Frequency Modulated Continuous Wave (FMCW). We will use the speaker from the tablet to transmit a periodic chirp and use its microphone to receive the reflected chirp simultaneously.

## How to Install and Run the Project

The main code of the project is in final. You will need to install the folder and modify gradle files to work on your computer. 

## How to Use the app

It is an Android app that needs two devices to operate. One device will act as a sender. In the main menu, you need to choose sender and click play chirp to create the chirp. The other device will act as a receiver. In the main menu you need to select receiver and click start when begin recording. After the chirp ends, click the button 'stop'. Then the app will show the FFT graph and also the estimated distance. If you think the distance is incorrect, you can also estimate the distance manually by selecting the interval between the first peak and the second peak.
