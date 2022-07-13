#!/bin/bash

declare -i player1_points=50
declare -i player2_points=50
declare -i ball_position=0
declare -i player1_guess=0
declare -i player2_guess=0

print_board() {
    echo " Player 1: ${player1_points}         Player 2: ${player2_points} "
    echo " --------------------------------- "
    echo " |       |       #       |       | "
    echo " |       |       #       |       | "
    case $ball_position in
        0 )
        echo  " |       |       O       |       | " ;;
        1 )
        echo " |       |       #   O   |       | " ;;
        2 )
        echo " |       |       #       |   O   | " ;;
        3 )
        echo " |       |       #       |       |O" ;;
        -1 )
        echo " |       |   O   #       |       | " ;;
        -2 )
        echo " |   O   |       #       |       | " ;;
        -3 )
        echo "O|       |       #       |       | " ;;
    esac
    echo " |       |       #       |       | "
    echo " |       |       #       |       | "
    echo " --------------------------------- "
}

print_players_guesses() {
    echo -e "       Player 1 played: ${player1_guess}\n       Player 2 played: ${player2_guess}\n\n"
}

make_players_choose() {
    echo "PLAYER 1 PICK A NUMBER: "
    read -s player1_temp_guess
    # While player1 guess is not a number or not between 0 and $player1_points
    while ! [[ $player1_temp_guess =~ ^[0-9]+$ ]] || [ $player1_temp_guess -lt 0 ] || [ $player1_temp_guess -gt $player1_points ]
    do
        echo "NOT A VALID MOVE !"
        echo "PLAYER 1 PICK A NUMBER: "
        read -s player1_temp_guess
    done
    
    echo "PLAYER 2 PICK A NUMBER: "
    read -s player2_temp_guess
    # While player2 guess is not a number or not between 0 and $player2_points
    while ! [[ $player2_temp_guess =~ ^[0-9]+$ ]] || [ $player2_temp_guess -lt 0 ] || [ $player2_temp_guess -gt $player2_points ]
    do
        echo "NOT A VALID MOVE !"
        echo "PLAYER 2 PICK A NUMBER: "
        read -s player2_temp_guess
    done
    
    player1_guess=$player1_temp_guess
    player2_guess=$player2_temp_guess
}

update_board() {
    # Update players points
    player1_points=$((player1_points-player1_guess))
    player2_points=$((player2_points-player2_guess))
    
    # If player1_points is negative, set it to 0
    if [ $player1_points -lt 0 ]
    then
        player1_points=0
    fi
    # If player2_points is negative, set it to 0
    if [ $player2_points -lt 0 ]
    then
        player2_points=0
    fi
    
    # Move the ball
    # If player1 won
    if [ $player1_guess -gt $player2_guess ]
    then
        # If ball_position is positive
        if [ $ball_position -gt 0 ]
        then
            ball_position=$((ball_position+1))
        else
            ball_position=1
        fi
        # If player2 won
    elif [ $player2_guess -gt $player1_guess ]
    then
        # If ball_position is negative
        if [ $ball_position -lt 0 ]
        then
            ball_position=$((ball_position-1))
        else
            ball_position=-1
        fi
    fi
}

check_if_game_is_over() {
    # If ball position is 3
    if [ $ball_position -eq 3 ]
    then
        echo "PLAYER 1 WINS !"
        exit 0
        # If ball position is -3
    elif [ $ball_position -eq -3 ]
    then
        echo "PLAYER 2 WINS !"
        exit 0
    fi
    
    # If player1 points is 0 and player2 points is positive
    if [ $player1_points -eq 0 ] && [ $player2_points -gt 0 ]
    then
        echo "PLAYER 2 WINS !"
        exit 0
        # If player2 points is 0 and player1 points is positive
    elif [ $player2_points -eq 0 ] && [ $player1_points -gt 0 ]
    then
        echo "PLAYER 1 WINS !"
        exit 0
    fi
    
    # If player1 points is 0 and player2 points is 0
    if [ $player1_points -eq 0 ] && [ $player2_points -eq 0 ]
    then
        # If ball position is positive
        if [ $ball_position -gt 0 ]
        then
            echo "PLAYER 1 WINS !"
            exit 0
            # If ball position is negative
        elif [ $ball_position -lt 0 ]
        then
            echo "PLAYER 2 WINS !"
            exit 0
        else
            echo "IT'S A DRAW !"
            exit 0
        fi
    fi
}

play_game() {
    print_board
    
    # While game is not over
    while true
    do
        make_players_choose
        update_board
        print_board
        print_players_guesses
        check_if_game_is_over
    done
}

play_game
