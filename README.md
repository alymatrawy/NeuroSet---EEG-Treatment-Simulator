Team 37:  
Brendan Bower  
Aly Matrawy  
Edwin Lau  
Matt Czarnowski  

Youtube demonstration video link: https://youtu.be/z4jCoe2tmPk

What each member worked on:

  - Brendan Bower
      - EEG waveform generation, EEG wave graphing. EEG testing buttons and functionality (Test Gamma, alpha, beta wave etc...), administering treatment, Electrode class and integrating the electrode class backend with the ui and mainwindow functionality. UML class diagram
    
  - Aly Matrawy
      - PC GUI and Buttons, saving data from session to a file, accessing date and time from file to show on session log, implemented therapy history viewing on pc, 2 Sequence Diagrams (Normal Operation of Treatment, Therapy History Viewing)
  - Edwin Lau
      - Device GUI and Buttons, New Session control flow, Time and Date settings, Use Cases, Use Case Diagram, State Diagram, 2 Sequence Diagrams (Low Battery Response, and Connection Loss)
  
  - Matt Czarnowski
      - "EEG Site Contact" UI and code, linking EEG site contact code with main device (disconnects, timer, etc), initial "external factors" panel and battery replacement code, requirements traceability matrix

Organization of the submission:
  - The directory where this README is in contains the source code
  - The 'Documentation' directory contains the documentation for the project
  - *NOTE*: QCustomPlot.h and QCustomPlot.cc are library files from the QCustomPlot library, they were not written by anyone in the group. They can be found on the QCustomPlot website. The class is used for graphing the EEG waveform within the PC interface.
