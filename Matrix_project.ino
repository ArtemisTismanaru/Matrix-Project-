#include <LedControl.h>
#include <LiquidCrystal.h>

bool isBombActive = false;
int highscore = 0;

bool inAboutMenu = false;  // Flag about Menu
int aboutSubMenuState = 0; 

bool inSettingsMenu = false;
int settingsSubMenuState = 0;
// Pin configurations for LCD
const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 3;
const byte d7 = 4;
const byte v0 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Pin configurations for components
const int buzzerPin = 8;
const int boomFrequency = 500;

// LED Matrix pin configuration
const int DIN_PIN = 12;
const int CLK_PIN = 11;
const int CS_PIN = 10;
LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, 1);

const int BOMB_LED_PIN = 8;
const int JOYSTICK_DEADZONE = 50;
const int MOVE_INTERVAL = 300;
const int BOMB_INTERVAL = 3000;
unsigned long lastMoveTime = 0;
unsigned long bombLastPlacedTime = 0;

const int MATRIX_SIZE = 8;

// Joystick and button pins
const int xPin = A0;
const int yPin = A1;
const int buttonPin = 2;

// Initial player position
int playerX = MATRIX_SIZE / 2;
int playerY = MATRIX_SIZE / 2;

// Matrix to represent walls
bool walls[MATRIX_SIZE][MATRIX_SIZE];
bool wallsGenerated = false; // Flag to check if walls have been generated
bool displayHiFlag = true;   // Flag to display a greeting message
unsigned long previousMillis = 0;
const long interval = 200;

// Bomb-related variables
bool bombPlaced = false;
bool detonateFlag = false;
int bombX, bombY;

bool flagBomb = false;

unsigned long bombPreviousMillis = 0;
const long bombInterval = 100;
int score = 0;

bool displayWelcome = true;

unsigned long timerGame;

bool displayFlagMenu = false;
int menuState = 0; // 0 - Main Menu, 1 - Settings Menu, 2 - About Menu
char selectedOption;

int currentMessageIndex = 0;

bool hiDisplayed = false;
bool buttonPressed = false;
bool gameStarted = false;
bool hiFlag = false;

unsigned long gameStartTime;

const char messages[][20] = {
  "BOMBERMAN"
};

bool displayAboutMenu = false;
const char* githubUsername = "tinutzaa";
const char* fullName = "Tina";

bool displaySettings = false;

int lcdBrightness = 128; // Initial LCD brightness value
int matrixBrightness = 10; // Initial matrix brightness value
int subMenuState = 0;

void setup() {
  Serial.begin(9600);

  // LED Matrix setup
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);

  lcd.begin(16, 2);
  pinMode(3, OUTPUT); // Contrast pin for LCD
  analogWrite(3, 128);

  // Component pins setup
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH);
  pinMode(BOMB_LED_PIN, OUTPUT);

  // Random seed based on analog input
  randomSeed(analogRead(A2));

  // Generate initial walls
  generateRandomWalls();

  // Record game start time
  gameStartTime = millis();
  analogWrite(v0, 100);
  displayWelcome = true;
}

bool inGame = false; 
bool inMenu = true;  

void loop() {
  // Display welcome messages and initialize the menu
  if (displayWelcome) {
    displayHi();
    displayScrollingMessage();
    displayFlagMenu = true;
    displayWelcome = false;
    hiDisplayed = false;
  }

  // Display the main menu and read joystick input

  if (displayFlagMenu && !inGame && !inSettingsMenu) {
    
    displayMenu();
    readJoystickMenu();
  } 

  // Game-specific logic

  if (inGame) {
    //Serial.print("Game is running:");
    handleGameplay();
    displayGameDetails();
  }

  // Handle settings menu
  if (inSettingsMenu) {
    displaySettingsMenu(); 
    readJoystickMenu();    
  }
  if (inAboutMenu) {
    displayAbout(); 
    readJoystickMenu();    
  }
}

void handleGameplay() {
  // Game logic including player movement and bomb handling
  if (!gameStarted) {
    startGameCountdown();
    gameStarted = true;
  }

  // Update and display game details (uncomment if needed)
  // displayGameDetails();

  // Check the joystick movement to move the player
  checkJoystickMovement();

  // Check if the joystick button is pressed to place a bomb
  checkBombPlacement();

  // Display walls, player, and bomb status
  displayWalls();
  displayBlinkingPlayer();
  displayBlinkingBomb();

  // Check the game reset condition (no walls left)
  checkGameResetCondition();
}

void startGameCountdown() {
  for (int count = 3; count >= 0; count--) {
    displayCountdown(count);
    delay(1000);
  }
}

void checkJoystickMovement() {
  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);

  if (abs(xValue - 512) > JOYSTICK_DEADZONE || abs(yValue - 512) > JOYSTICK_DEADZONE) {
    if (millis() - lastMoveTime >= MOVE_INTERVAL || bombPlaced) {
      updatePlayerPosition(xValue, yValue);
      lastMoveTime = millis();
    }
  }
}

void checkBombPlacement() {
  if (digitalRead(buttonPin) == LOW) {
    //Serial.println("Button pressed");
    if (!bombPlaced && wallsGenerated) {
      placeBomb();
      //Serial.println("Bomb placed");
    }
  } else if (millis() - bombLastPlacedTime > BOMB_INTERVAL && wallsGenerated) {
      
      detonateBomb();
      updateScore();
      handleButtonPress();
    }
  
}

void checkGameResetCondition() {
  if (noWallsLeft()) {
    resetGame();
  }
  updateHighscore();
}

void resetGame() {
  wallsGenerated = false;
  bombPlaced = false;
  detonateFlag = false;
  inGame = false;  // Reset the in-game flag
  showMenu();
}


void showMenu() {
  inGame = false;
  inMenu = true;
  displayFlagMenu = true;
  lcd.clear(); 
  // 
}

void adjustLcdBrightness(int value) {
  lcdBrightness = constrain(value, 5, 255);
  analogWrite(5, lcdBrightness);
}

void adjustMatrixBrightness(int value) {
  matrixBrightness = constrain(value, 0, 15);
  lc.setIntensity(0, matrixBrightness);
}

void startGame() {
  // Initialize game logic and countdown
  displayCountdown(3);
  generateRandomWalls();
  wallsGenerated = true;
  displayHiFlag = false;
  score = 0;
  bombPlaced = false;
  detonateFlag = false;
  gameStartTime = millis();
  buttonPressed = true;
  inGame = true;  // Set the game flag to true
  lcd.clear();   // Clear the LCD screen at the start of the game
}

void displayMessage() {
  // Clear the LCD screen and set the cursor to the top-left position
  //lcd.clear();
  lcd.setCursor(0, 0);

  // Print the current message to the LCD
  lcd.print(messages[currentMessageIndex]);

  // Enable autoscroll to display long messages on a single line
  lcd.autoscroll();

  // Stop autoscroll after a delay to allow reading the message
  delay(1000);
  lcd.noAutoscroll();

  // Stop displaying messages after the last message
  if (currentMessageIndex < sizeof(messages) / sizeof(messages[0]) - 1) {
    // Move to the next message index if there are more messages
    currentMessageIndex++;
  } else {
    // Set flags to control the flow (welcome done, flag menu displayed)
    displayWelcome = false;
    displayFlagMenu = true;
    displayHiFlag = false;
  }
}

void displayScrollingMessage() {

  const char *message = "Welcome to the Jungle          ";
  int lcdWidth = 16;  // Number of characters displayed per line on the LCD
  int messageLength = strlen(message);

  // Iterate through the message and scroll it on the LCD
  for (int i = 0; i < messageLength + lcdWidth; ++i) {
    //lcd.clear();
    lcd.setCursor(0, 0);

    // Display a portion of the message based on the current iteration
    for (int j = 0; j < lcdWidth; ++j) {
      int index = (i + j) % messageLength;
      lcd.write(message[index]);
    }

    delay(200);  // Wait for 200 milliseconds between each movement
  }

  delay(1000);  // Wait for 1 second after scrolling
  displayMessage();  // Move to the next step: displaying messages
  //displayWelcome = false;
}

void updateHighscore() {
  if (score > highscore) {
    highscore = score; // Actualizează highscore-u
  }
}

void displayHighscore() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Highscore:");
  lcd.setCursor(0, 1);
  lcd.print(highscore);
}

void displayAbout() {
  inAboutMenu = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("About");

  lcd.setCursor(0, 1);
  switch (aboutSubMenuState) {
    case 0:
      lcd.print("a) about game");
      break;
    case 1:
      lcd.print("b) highscore");
      break;
    case 2:
      lcd.print("Back");
      break;
  }
}


void displaySettingsMenu() {
  inSettingsMenu = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SETTINGS");

  lcd.setCursor(0, 1);
  switch (settingsSubMenuState) {
    case 0:
      lcd.print("a) LCD Brightness");
      break;
    case 1:
      lcd.print("b) Matrix Brightness");
      break;
    case 2:
      lcd.print("Back");
      break;
  }
  //lcd.clear();
}



bool inMainMenu = true;



void displayMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MAIN MENU");
  lcd.setCursor(0, 1);
  switch (menuState) {
    case 0:
      lcd.print("> 1. Start Game");
      break;
    case 1:
      lcd.print("> 2. Settings");
      break;
    case 2:
      lcd.print("> 3. About");
      break;
  }
}


/*void adjustLcdBrightness(int value) {
  lcdBrightness = constrain(value, 0, 255);
  analogWrite(3, lcdBrightness);
}

void adjustMatrixBrightness(int value) {
  matrixBrightness = constrain(value, 0, 15);
  lc.setIntensity(0, matrixBrightness);
}
*/
void readJoystickMenu() {
  int joystickX = analogRead(xPin);  
  int joystickY = analogRead(yPin);  

  // Check joystick movement on the Y axis
  if (joystickY < 100) {
    // Move up in the menu
    if (displayAboutMenu) {
      displayAboutMenu = false;
    } else {
      menuState = (menuState - 1 + 3) % 3;
      displayMenu();
      //delay(300);
    }
  } else if (joystickY > 900) {
    // Move down in the menu
    if (displayAboutMenu) {
      displayAboutMenu = false;
    } else {
      menuState = (menuState + 1) % 3;
      displayMenu();
      //delay(300);
    }
  }
  
  // Check joystick button press
  if (digitalRead(buttonPin) == LOW) {
    if(!inSettingsMenu && displayFlagMenu){
      switch (menuState) {
        case 0:
          //logic to start game
          startGame();
          inGame = true;
          displayFlagMenu = false;
          break;
        case 1:
          //displayFlagMenu = false;
          inSettingsMenu = true;
          settingsSubMenuState = 0;
          // logic for settings
          displaySettingsMenu();
          break;
        case 2:
          displayFlagMenu = false;
          inAboutMenu = true;
          aboutSubMenuState = 0;
          // Logic for abouy
          displayAbout();
          break;
      }
    }else if(inSettingsMenu){
        switch (settingsSubMenuState) {
        case 0:
          
          break;
        case 1:
          
          break;
        case 2:
          // Exit from settings
          inSettingsMenu = false;
          displayMenu();
          displayFlagMenu = true;
          break;
      }
    }else if (inAboutMenu){
      switch (aboutSubMenuState) {
      case 0:
        if (digitalRead(buttonPin) == LOW) {
        displayAboutGameDetails();
        delay(500);  
        }
        break;
      case 1:
        if (digitalRead(buttonPin) == LOW) {
        displayHighscore();
        delay(500);  
        }
        break;
      case 2:
        //show the highscore
        inAboutMenu = false;
        displayMenu();
        displayFlagMenu = true;
        break;
    }
    }
    delay(200);
  }

  if (inSettingsMenu) {
    if (joystickY < 100) {
      settingsSubMenuState = (settingsSubMenuState - 1 + 2) % 3;
      displaySettingsMenu();
      delay(200);  
    } else if (joystickY > 900) {
      settingsSubMenuState = (settingsSubMenuState + 1) % 3;
      displaySettingsMenu();
      delay(200);  
    }

    if (settingsSubMenuState == 0) {  // if we are in submenu "LCD Brightness"
            if (joystickX < 100 || joystickX > 900) {
                int brightnessChange = map(joystickX, 0, 1023, -5, 5);
                lcdBrightness = constrain(lcdBrightness + brightnessChange, 0, 255);
                adjustLcdBrightness(lcdBrightness);

                // show the new value
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("LCD Brightness:");
                lcd.setCursor(0, 1);
                lcd.print(lcdBrightness);
                delay(100);  
            }
        }
    if (settingsSubMenuState == 2 && digitalRead(buttonPin) == LOW) { 
            inSettingsMenu = false;
            displayFlagMenu = true;
            lcd.clear();
            displayMenu();
    }
    if (settingsSubMenuState == 1) {  
            if (joystickX < 100 || joystickX > 900) {
                int brightnessChange = map(joystickX, 0, 1023, -1, 1);
                matrixBrightness = constrain(matrixBrightness + brightnessChange, 0, 15);
                adjustMatrixBrightness(matrixBrightness);

                // show the new value
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Matrix Bright:");
                lcd.setCursor(0, 1);
                lcd.print(matrixBrightness);
                delay(100);  
        }

  }
  // Check if the settings menu is being displayed
  if (inAboutMenu) {
    if (joystickY < 100) {
      aboutSubMenuState = (aboutSubMenuState - 1 + 2) % 3;
      displayAbout();
      delay(200);  
    } else if (joystickY > 900) {
      aboutSubMenuState = (aboutSubMenuState + 1) % 3;
      displayAbout();
      delay(200);  
    }
  }

}


void displayCountdown(int count) {
  hiFlag = true;
  const uint64_t countdownImage[] = {
    0x0000000000000000,
    //
    0x0018181c1818187e, // 1
    0x003c6660300c067e, // 2
    0x003c66603860663c, // 3
  };

  if (count >= 0 && count <= 3) {
    displayImage(countdownImage[count]);
    //delay(500);
    //lc.clearDisplay(0);
  }
}


void handleButtonPress() {
  // Show to the display the countdown images and make decisions 
  if (digitalRead(buttonPin) == LOW && !wallsGenerated ) {
    for (int count = 3; count >= 0; count--) {
      displayCountdown(count);
    }
    wallsGenerated = true;  
  }
}

int lastDisplayedTime = -1;
int lastDisplayedScore = -1;

void displayAboutGameDetails() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Name: Tina");
  lcd.setCursor(0, 1);
  lcd.print("GitHub: Tinutza");
}


void displayGameDetails() {
  if (isBombActive) {
    return; 
  }

  unsigned long elapsedTime = millis() - gameStartTime;
  int seconds = elapsedTime / 1000;
  lcd.setCursor(0, 0);
  lcd.print("Time: ");

  
  if (seconds != lastDisplayedTime) {
    lcd.setCursor(6, 0); // Set cursor to right after "Time: "
    if (seconds < 10) {
      lcd.print("   "); // Clear the previous characters
    } else if (seconds < 100) {
      lcd.print("  "); // Clear less characters for two-digit numbers
    }
    lcd.setCursor(6, 0); // Reset cursor to position after "Time: "
    lcd.print(seconds);
    lcd.print("s");
    lastDisplayedTime = seconds;
  }
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  //lcd.clear();
  if (score != lastDisplayedScore) {
    lcd.setCursor(7, 1); // Set cursor to right after "Score: "
    if (score < 10) {
      lcd.print("   "); // Clear the previous characters
    } else if (score < 100) {
      lcd.print("  "); // Clear less characters for two-digit numbers
    }
    lcd.setCursor(7, 1); // Reset cursor to position after "Score: "
    lcd.print(score);
    lastDisplayedScore = score;
  }
}

// Function to check if the player is surrounded by walls
bool isPlayerSurrounded(int x, int y) {
  if ((x > 0 && walls[y][x - 1]) || (x < MATRIX_SIZE - 1 && walls[y][x + 1]) ||
      (y > 0 && walls[y - 1][x]) || (y < MATRIX_SIZE - 1 && walls[y + 1][x])) {
    return true;
  }
  return false;
}

// Function to play a sound when a bomb detonates
void playBoomSound() {
  tone(buzzerPin, boomFrequency, 1000);
  delay(500);
  noTone(buzzerPin);
}

// Function to generate random walls
void generateRandomWalls() {
  for (int i = 0; i < MATRIX_SIZE; i++) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
      walls[i][j] = random(2) == 0;
    }
  }
  
  do{
  playerX = random(MATRIX_SIZE);
  playerY = random(MATRIX_SIZE);
  }while(walls[playerY][playerX] || isPlayerSurrounded(playerX, playerY));
  gameStartTime = millis();
}

// Function to display walls on the LED Matrix
void displayWalls() {
  for (int i = 0; i < MATRIX_SIZE; i++) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
      if (walls[i][j]) {
        lc.setLed(0, i, j, true);
      }
    }
  }

}

// Function to display blinking player on the LED Matrix
void displayBlinkingPlayer() {
  unsigned long currentMillis = millis();
  static bool playerLEDState = false;

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    playerLEDState = !playerLEDState;

    lc.setLed(0, playerY, playerX, playerLEDState);
  }
}

// Function to display blinking bomb on the LED Matrix
void displayBlinkingBomb() {
  unsigned long currentMillis = millis();
  static bool bombLEDState = false;

  if (bombPlaced && detonateFlag) {
    if (currentMillis - bombPreviousMillis >= bombInterval) {
      bombPreviousMillis = currentMillis;
      bombLEDState = !bombLEDState;

      lc.setLed(0, bombY, bombX, bombLEDState);
    }
  }
}

// Function to update player position based on joystick input
void updatePlayerPosition(int xValue, int yValue) {
  int deltaX = xValue - 512;
  int deltaY = yValue - 512;

  int previousX = playerX;
  int previousY = playerY;

  lc.setLed(0, previousY, previousX, false);

  unsigned long currentMillis = millis();

  if (currentMillis - lastMoveTime >= MOVE_INTERVAL) {
    if (abs(deltaX) > JOYSTICK_DEADZONE) {
      if (deltaX > 0 && playerX < MATRIX_SIZE - 1 && !walls[playerY][playerX + 1]) {
        playerX++;
      } else if (deltaX < 0 && playerX > 0 && !walls[playerY][playerX - 1]) {
        playerX--;
      }
    }

    if (abs(deltaY) > JOYSTICK_DEADZONE) {
      if (deltaY > 0 && playerY < MATRIX_SIZE - 1 && !walls[playerY + 1][playerX]) {
        playerY++;
      } else if (deltaY < 0 && playerY > 0 && !walls[playerY - 1][playerX]) {
        playerY--;
      }

      if (playerX != previousX || playerY != previousY) {
        lc.setLed(0, playerY, playerX, true);
        flagBomb = true;
      }

      lastMoveTime = currentMillis;
    }
  }
}

// Function to check if there are no walls left
bool noWallsLeft() {
  for (int i = 0; i < MATRIX_SIZE; i++) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
      if (walls[i][j]) {
        return false;
      }
    }
  }
  return true;
}

// Function to place a bomb
void placeBomb() {

  //Serial.print("placing Bomb");
  if (!bombPlaced) {
    bombX = playerX;
    bombY = playerY;
    bombPlaced = true;
    detonateFlag = true;
    digitalWrite(BOMB_LED_PIN, HIGH);
    bombLastPlacedTime = millis();
  }
  isBombActive = true;
  lcd.clear();
}

// Function to detonate a bomb and eliminate nearby walls
void detonateBomb() {
  Serial.println("Detonating bomb");
  if (bombPlaced) {
    eliminateFirstWallInDirection(bombX, bombY, 1, 0);
    eliminateFirstWallInDirection(bombX, bombY, -1, 0);
    eliminateFirstWallInDirection(bombX, bombY, 0, 1);
    eliminateFirstWallInDirection(bombX, bombY, 0, -1);

    lc.setLed(0, bombY, bombX, false);
    digitalWrite(BOMB_LED_PIN, LOW);
    bombPlaced = false;
    detonateFlag = false;
    playBoomSound();

    score += (millis() - bombLastPlacedTime) / 1000;
  }
  //lcd.begin(16, 2);
  displayGameDetails(); 
  isBombActive = false;
}

// Function to eliminate the first wall in a given direction
void eliminateFirstWallInDirection(int startX, int startY, int dirX, int dirY) {
  for (int x = startX + dirX, y = startY + dirY;
       x >= 0 && x < MATRIX_SIZE && y >= 0 && y < MATRIX_SIZE;
       x += dirX, y += dirY) {
    if (walls[y][x]) {
      walls[y][x] = false;
      lc.setLed(0, y, x, false);
      break;
    }
  }
}

void displayImage(uint64_t image) {
  for (int i = 0; i < 8; i++) {
    byte rowData = (image >> (i * 8)) & 0xFF;
    lc.setRow(0,7- i, rowData);
  }
}

// Function to display a greeting message
void displayHi() {
  const uint64_t HI_IMAGE = 0x005212525e525200;

  unsigned long currentMillis = millis();
  if (millis() - currentMillis < interval && !hiDisplayed) {
    displayImage(HI_IMAGE);
    if(hiFlag)
      lc.clearDisplay(0);
    hiDisplayed = true;
  }
}

// Function to update and display the score
void updateScore() {
  Serial.print("Score: ");
  Serial.println(score);
}