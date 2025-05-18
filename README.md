# 🏴 Treasure Hunt Hub

[![Build Status](https://img.shields.io/github/actions/workflow/status/username/treasure-hub/ci.yml)]()
[![Coverage](https://img.shields.io/codecov/c/github/username/treasure-hub)]()
[![License](https://img.shields.io/github/license/username/treasure-hub)]()
[![Version](https://img.shields.io/github/v/tag/username/treasure-hub)]()

## 🎬 Demo

![CLI Demo](docs/demo.gif)

## 📋 Short Presentation
Treasure Hunt Hub este o aplicație CLI pentru organizarea vânătorilor de comori:

  -procese multiple comunicate prin pipe-uri și semnale

  -comandă externă calculate_score

  -interfață interactivă pentru administrare

## ✨ Example & Compile method

  1. gcc -Wall -o treasure_manager treasure_manager.c
  2. gcc -Wall -o calculate_score calculate_score.c
  3. gcc -Wall -o prg hub.c
  4. ./prg

=== Starting monitor ===
Monitor has started with PID: <pid_number>

=== HUB MENU ===
1) List all hunts
2) List treasures in a hunt
3) View a treasure by ID
4) Calculate scores for a hunt
5) Stop monitor and exit
Select: <option> ex 1

hunt_alpha: 3 treasures
hunt_beta: 5 treasures

Select: <option> ex 4
Hunt name: hunt_alpha
Scores for hunt 'hunt_alpha':
  alice: 120
  bob:   80

Select: 5
Stopping monitor...
Monitor exited normally (code 255)
