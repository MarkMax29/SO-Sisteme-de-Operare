# 🏴 Treasure Hunt Hub

## 📋 Short Presentation

Treasure Hunt Hub este o aplicație CLI pentru organizarea vânătorilor de comori:

  -procese multiple comunicate prin pipe-uri și semnale

  -comandă externă calculate_score

  -interfață interactivă pentru administrare

###FILES

 Phase 1 : found in the treasure_manager.c file and it's compiled with treasure_manager<br>
 Phase 2 : found in the hub.c file and it's compiled with prg<br>
 Phase 3 : foun in the calculate_score.c file as well as in the hub.c which has modifications for this phase. the calculate_score.c is compiled with calculate_score

## ✨ Example & Compile method

  1. gcc -Wall -o treasure_manager treasure_manager.c
  2. gcc -Wall -o calculate_score calculate_score.c
  3. gcc -Wall -o prg hub.c
  4. ./prg

### EXAMPLE 
=== Starting monitor ===

Monitor has started with PID: <pid_number>

=== HUB MENU ===
1) List all hunts
2) List treasures in a hunt
3) View a treasure by ID
4) Calculate scores for a hunt
5) Stop monitor and exit<br>
Select: <option> ex 1

hunt_alpha: 3 treasures<br>
hunt_beta: 5 treasures

Select: <option> ex 4<br>
Hunt name: hunt_alpha<br>
Scores for hunt 'hunt_alpha':<br>
  alice: 120<br>
  bob:   80

Select: 5<br>
Stopping monitor...<br>
Monitor exited normally (code 255)

## 🚍 Credits

  This project was made by Horvath Mark.<br>
  This project may suffer slight modifications.