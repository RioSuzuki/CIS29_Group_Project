#include <iostream>
#include <cassert>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Collision.h"
#include "Game.h"
#include "Player.h"
#include "Projectile.h"
#include "ToolBar.hpp"
#include "HealthRestore.h"
#include "Shield.h"
#include "Menu.h"
#include "Options.h"
#include "ResourceNotLoaded.h"
#include "Endscreen.h"
#include "InputHighscore.h"

using namespace PlanetDefenders;

void Game::loadAllMusic()
{
    // add exception handling here
    if (!titleThemeBuffer.loadFromFile(AudioBasePath + "titleTheme.ogg"))
        throw MusicNotLoaded("titleTheme.ogg");
    else
        backgroundMusic.setBuffer(titleThemeBuffer);
    if (!backgroundBuffer.loadFromFile(AudioBasePath + "game_music.ogg"))
        throw MusicNotLoaded("game_music.ogg");
    if (!laserSoundBuffer.loadFromFile(AudioBasePath + "laser.ogg"))
        throw MusicNotLoaded("laser.ogg");
    else
        laserSound.setBuffer(laserSoundBuffer);

    if (!powerupSoundBuffer.loadFromFile(AudioBasePath + "enemy_hurt.ogg"))
        throw MusicNotLoaded("enemy_hurt.ogg");
    else
        enemyHurtSound.setBuffer(powerupSoundBuffer);

    if (!enemyHurtSoundBuffer.loadFromFile(AudioBasePath + "powerup.ogg"))
        throw MusicNotLoaded("powerup.ogg");
    else {
        powerupSound.setBuffer(enemyHurtSoundBuffer);
        powerupSound.setVolume(40);
    }
}

void Game::pauseGame()
{
    if (GAME_PAUSED)
        backgroundMusic.pause();
    else
        backgroundMusic.play();
}

bool Game::menuHandleKeyboard(sf::Event& event)
{
    //using left and right keys to switch between the options
    switch (event.key.code) {
    case sf::Keyboard::Left:
        menu->MoveLeft();
        break;
    case sf::Keyboard::Right:
        menu->MoveRight();
        break;
    case sf::Keyboard::Return:
        switch (menu->getPressedItem()) {
        case 0:
            std::cout << "Play button has been pressed" << std::endl;
            return true;
            break;
        case 1:
            std::cout << "Options button has been pressed" << std::endl;
            // to options view
            options->work(*window, event, *options, GameBackground, backgroundMusic);
            break;
        case 2:
            std::cout << "Exit button has been pressed" << std::endl;
            window->close();
            return true;
            break;
        }
    }
    return false;
}

bool Game::menuHandleMouseClicked(sf::Event& event)
{
    if (menu->inY(event, *window)) {
        if (menu->inXPlay(event, *window)) {
            std::cout << "PLAY";
            return true;
        }
        else if (menu->inXOptions(event, *window)) {
            std::cout << "Options";
            // to options view
            options->work(*window, event, *options, GameBackground, backgroundMusic);
        }
        else if (menu->inXExit(event, *window)) {
            std::cout << "Exit";
            window->close();
            return true;
        }
    }
    return false;
}

bool Game::menuHandleMouseMove(sf::Event& event)
{
    if (menu->inYHover(event, *window)) {
        if (menu->inXPlayHover(event, *window)) {
            menu->clickMenu();
        }
        else if (menu->inXOptionsHover(event, *window)) {
            menu->clickOptions();
        }
        else if (menu->inXExitHover(event, *window)) {
            menu->clickExit();
        }
    }
    return false;
}


/*
    return true to leave the menu and go back to main game loop
    return false keep showing menu
*/
bool Game::displayMenu()
{
    sf::Event event;
    while (window->pollEvent(event)) {
        switch (event.type) {
        case sf::Event::KeyPressed:
            if (menuHandleKeyboard(event))
                return true;
            //Clicking on a button
        case sf::Event::MouseButtonReleased:
            if (menuHandleMouseClicked(event))
                return true;
            //Changing colors when hovering over the buttons
        case sf::Event::MouseMoved:
            menuHandleMouseMove(event);
            break;
        case sf::Event::Closed:
            window->close();
            return true;
            break;
        }
        window->draw(GameBackground);
        menu->draw(*window);
    }
    return false;
}

void Game::collisionPlayerProjAndBoss()
{
    // Collision between playerProjectile and enemy
    for (int i = 0; i < playerProjectileArray.size(); i++)
    {
        //if (checkCollision(playerProjectileArray[i], boss))
        if (playerProjectileArray[i]->collide(*dynamic_cast<GameObject*>(boss)))
        {
            tool->addScore(1);
            enemyHurtSound.play();
            bossHp--;
            std::cout << "PROJECTILE COLIDED BOSS" << std::endl;
            deleteObjectFromVector(playerProjectileArray, i);
            break;
        }
    }
}

void Game::collisionBossProjAndPlayer()
{
    for (int i = 0; i < bossProjectileArray.size(); i++)
    {
        //if (checkCollision(bossProjectileArray[i], player))
        if (bossProjectileArray[i]->collide(*dynamic_cast<GameObject*>(player)))
        {
            //enemyHurtSound.play();  playerHurtSound?
            player->takeDamage(bossProjectileArray[i]->getDamage());

            std::cout << "BOSS PROJECTILE COLIDED PLAYER" << std::endl;
            deleteObjectFromVector(bossProjectileArray, i);
            break;
        }
    }
}

void Game::collisionPlayerProjAndEnemy()
{
    // Collision between playerProjectile and enemy
    for (int i = 0; i < playerProjectileArray.size(); i++)
    {
        for (int j = 0; j < enemyArr.size(); j++)
        {
            //if (checkCollision(playerProjectileArray[i], enemyArr[j]))
            if (playerProjectileArray[i]->collide(*dynamic_cast<GameObject*>(enemyArr[j])))
            {
                tool->addScore(2);
                enemyHurtSound.play();
                deleteObjectFromVector(playerProjectileArray, i);
                deleteObjectFromVector(enemyArr, j);
                std::cout << "PROJECTILE COLIDED ENEMY" << std::endl;
                break;
            }
        }
    }
}

void Game::collisionEnemyProjAndShield()
{
    // Collision between enemyProjectile player shield
    for (int i = 0; i < enemyProjectileArray.size(); i++)
    {
        //if (checkCollision(&enemyProjectileArray[i]->getSprite(), &shieldSprite))
        if (enemyProjectileArray[i]->collide(shieldSprite))
        {
            //player->takeDamage(1);
            //tool->updateHpBarSize(player->getHp());
            deleteObjectFromVector(enemyProjectileArray, i);
            break;
        }
    }
}

void Game::collisionBossProjAndShield()
{
    for (int i = 0; i < bossProjectileArray.size(); i++)
    {
        //if (checkCollision(&bossProjectileArray[i]->getSprite(), &shieldSprite))
        if (bossProjectileArray[i]->collide(shieldSprite))
        {
            deleteObjectFromVector(bossProjectileArray, i);
            break;
        }
    }
}

void Game::collisionEnemyProjAndPlayer()
{
    for (int i = 0; i < enemyProjectileArray.size(); i++)
    {
        //if (checkCollision(enemyProjectileArray[i], player))
        if (player->collide(*dynamic_cast<GameObject*>(enemyProjectileArray[i])))
        {
            player->takeDamage(enemyProjectileArray[i]->getDamage());
            deleteObjectFromVector(enemyProjectileArray, i);
            break;
        }
    }

}

void Game::collisionEnemyAndPlayer()
{
    for (int i = 0; i < enemyArr.size(); i++)
    {
        //if (checkCollision(player, enemyArr[i]))
        if (player->collide(*dynamic_cast<GameObject*>(enemyArr[i])))
        {
            std::cout << "PLAYER COLIDED ENEMY" << std::endl;
            //player->takeDamage(1);
            player->takeDamage(1);
            break;
        }
    }

}

void Game::collisionPowerUpAndPlayer()
{
    for (int i = 0; i < powerUpArr.size(); i++)
    {
        //if (checkCollision(player, powerUpArr[i]))
        if (player->collide(*dynamic_cast<GameObject*>(powerUpArr[i])))
        {
            std::cout << "PLAYER COLIDED POWERUP" << std::endl;
            // do not delete powerUp in GameLoop
            player->addPowerUp(dynamic_cast<PowerUp*>(powerUpArr[i]));
            tool->setPowerUp(
                dynamic_cast<PowerUp*>(powerUpArr[i])->getType(),
                dynamic_cast<PowerUp*>(powerUpArr[i])->getDuration()
            );
            std::swap(powerUpArr[i], powerUpArr.back());
            powerUpArr.pop_back();
            powerupSound.play();
            break;
        }
    }
}

void Game::enemyRandomShoot()
{
    for (int i = 0; i < enemyArr.size(); i++)
    {
        Projectile* newProjectile = dynamic_cast<Enemy*>(enemyArr[i])->shoot();
        if (newProjectile)
        {
            newProjectile->roateToDirection();
            enemyProjectileArray.push_back(newProjectile);
        }
    }

}

void Game::bossRandomShoot()
{
    //int randShoot = random() % 4;
    int randShoot = 0;
    std::vector<Projectile*> newProjectile;
    // v1 = boss position + boss bottom middle posision
    const static sf::Vector2f v1 =
        sf::Vector2f(boss->getBound().width / 2.0f, boss->getBound().height) +
        boss->getPosition();
    const static sf::Vector2f offset =
        sf::Vector2f(player->getBound().width / 2.0f, player->getBound().height / 2.0f);
    switch (randShoot) {
    case 0:
        for (int i = 0; i < 4; i++) {   // shoot for time each times with i projectile?
            newProjectile = *boss->shoot(i);
            //for (auto it = newProjectile.begin(); it != newProjectile.end(); it++)
            for (auto& proj : newProjectile)
            {
                // vector with the direction to the player = player position - v1
                proj->setDirection(normalize(player->getPosition() - v1 + offset));
                proj->roateToDirection();
            }
            bossProjectileArray.insert(
                bossProjectileArray.end(),
                newProjectile.begin(),
                newProjectile.end()
            );
        }
        break;
    }
}

// return the first index of the newly added enemy in enemyArr
int Game::generateDiagonalEnemy(int n, sf::Vector2f initialPos, sf::Vector2f direction) {
    int startIndex = enemyArr.size();
    for (int i = 0; i < n; i++) {
        enemyArr.push_back(new Enemy(
            SPACE_TEXTURE,
            enemyRectArr[rand() % 2],
            sf::Vector2f(
                initialPos.x + i * direction.x * 20.f,
                initialPos.y + i * direction.y * 26.f)
        ));
    }
    return startIndex;
}

// return the first index of the newly added enemy in enemyArr
int Game::generateSquqreEnemy(int row, int col, sf::Vector2f initialPos, sf::Vector2f direction) {
    int startIndex = enemyArr.size();
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            enemyArr.push_back(new Enemy(
                SPACE_TEXTURE,
                enemyRectArr[rand() % 2],
                sf::Vector2f(
                    initialPos.x + i * 40.f,
                    initialPos.y + j * 30.f)
            ));
        }
    }
    return startIndex;
}

void Game::generateEnemy() {
    int time = genEnemyClock.getElapsedTime().asSeconds();
    randomEnemy = 1 + rand() % 5;       // num of enemy
    //initialPos = sf::Vector2f(rand() % 900, -100);
    initialPos = sf::Vector2f(-100, 200);
    float randX = rand() % 8 - 4;
    float randY = rand() % 8 - 4;
    direction = sf::Vector2f(randX, randY);
    int randNum = 1 + rand() % 4;
    int startIndex = 0;
    if (time > 2)           // every 2 seconds
    {
        if (tool->getTime().getElapsedTime().asSeconds() < 30)
            startIndex = generateDiagonalEnemy(randomEnemy, initialPos, direction);
        else {
            if (randNum % 2 == 0)
                startIndex = generateDiagonalEnemy(randomEnemy, initialPos, direction);
            else
                startIndex = generateSquqreEnemy(abs(randX), abs(randY), initialPos, direction);
        }
        genEnemyClock.restart();
        // set the enemy from first index to the last element
        for (int i = startIndex; i < enemyArr.size(); i++) {
            enemyArr[i]->setDirection(sf::Vector2f(1, 0));
            enemyArr[i]->setSpeed(randNum);
        }
    }
}

Game::Game()
{
    shipNum = 0; //ADD SOMETHING IN MENU TO SELECT SHIP (0 to 3)

    menu = new Menu(WindowWidth, WindowHeight);
    options = new Options(WindowWidth, WindowHeight);
    endscreen = new Endscreen(WindowWidth, WindowHeight);
    inputHighscore = new InputHighscore(WindowWidth, WindowHeight);

    // window setup
    window = new sf::RenderWindow(sf::VideoMode(WindowWidth, WindowHeight), GameTitle, sf::Style::Close | sf::Style::Resize);
    //std::cout << window->getSize().x << " " << window->getSize().y << std::endl;
    window->setFramerateLimit(FRAME_RATE_LIMIT);

    // sounds setup
    try
    {
        loadAllMusic();
    }
    catch (const MusicNotLoaded & exception)
    {

        exit(-1);
    }

    backgroundMusic.setVolume(20.0f);
    backgroundMusic.setLoop(true);

    // base texture setup
    if (!SPACE_TEXTURE.loadFromFile(TextureBasePath + "spaceSprites.png"))
        throw TextureNotLoaded("spaceSprites.png");
    if (!BackgroundTexture.loadFromFile(TextureBasePath + "spaceBackground.png"))
        throw TextureNotLoaded("spaceBackground.png");
    if (!ToolBarBackgroundTexture.loadFromFile(TextureBasePath + "toolbar.png"))
        throw TextureNotLoaded("toolbar.png");

    GameBackground = sf::Sprite(BackgroundTexture);
    sf::Vector2u TextureSize = BackgroundTexture.getSize();
    sf::Vector2u WindowSize = window->getSize();
    float ScaleX = (float)WindowSize.x / TextureSize.x;
    float ScaleY = (float)WindowSize.y / TextureSize.y;
    GameBackground.setScale(ScaleX, ScaleY);

    // ToolBar
    ToolBarBackground = sf::Sprite(ToolBarBackgroundTexture, sf::IntRect(0, 0, 204, 720));
    ToolBarBackground.setPosition(sf::Vector2f(1076, 0));
    tool = new ToolBar(sf::Vector2f(980, 0));
    assert(tool);           // Make sure tool is not null

    player = new Player(SPACE_TEXTURE, SHIP_TEXTURE_RECT[shipNum], sf::Vector2f(100, 100), shipNum);
    player->setMovingBoundary(sf::Vector2u(1076, 720));      // set moving bound so that it wont go over to ToolBar
    player->getSprite().scale(sf::Vector2f(1, 1) * 1.5f);
    tool->updateHpBarSize(player->getHp() / SHIP_MAX_HP[shipNum]); //send percentage of health

    // demo
    // build enemies array
    //initEnemy(window->getSize());

    enemyRectArr.push_back(EnemyRectEye);
    enemyRectArr.push_back(EnemyRectBlue);

    boss = new Boss(SPACE_TEXTURE, EnemyRectBoss, sf::Vector2f((window->getSize().x - tool->getSize().x) / 2, 0));
    //bossHp = -100;
    boss->setBossHpBar(bossHp, sf::Color::Red, sf::Vector2f(33.f, (float)bossHp * 7.2f), sf::Vector2f(0, 720.f));
    shieldSprite = sf::Sprite(SPACE_TEXTURE);
    shieldSprite.setTextureRect(sf::IntRect(134, 0, 45, 45));
    shieldSprite.setScale(1.4, 1.4);

    bossSprite = sf::Sprite(ToolBarBackgroundTexture, sf::IntRect(0, 720, 204, 64));
    bossSprite.setPosition(sf::Vector2f(1076.f, 581.0f));

}


void Game::handleKeyInput()
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        player->move(sf::Vector2f(-SHIP_SPEED[shipNum], 0.0f));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        player->move(sf::Vector2f(SHIP_SPEED[shipNum], 0.0f));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        player->move(sf::Vector2f(0.0f, -SHIP_SPEED[shipNum]));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        player->move(sf::Vector2f(0.0f, SHIP_SPEED[shipNum]));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
    {
        // demo
        // press R the accerlerate
        player->accelerate(1.1);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    {
        Projectile* newProjectile = player->shoot();
        if (newProjectile) {
            playerProjectileArray.push_back(newProjectile);
            laserSound.play();
        }
    }
}

void Game::handleBackdoorKeyInput(sf::Keyboard::Key code)
{
    switch (code)
    {
    case InfinityHpKey:
        InfinityHpTriggered = !InfinityHpTriggered;
        break;
    case AccelerateKey:
        player->accelerate(5);
        break;
    case DeaccelerateKey:
        player->deaccelerate(5);
        break;
    case BiggerProjectile:
        BiggerProjTriggered = !BiggerProjTriggered;
        if (BiggerProjTriggered)
        {
            player->setBackdoorProjScale(5.0f);
            player->setBackdoorProjDamage(5.0f);
        }
        else
        {
            // TODO Add to resetGame
            player->setBackdoorProjScale(1.0f);
            player->setBackdoorProjDamage(1.0f);
        }
    }

}

void Game::generatePowerUp()
{
    int numberOfPowerUpTypes = 2;
    if (genPowerUpClock.getElapsedTime().asSeconds() > 8)
    {
        PowerUpType randomPowerUpType = static_cast<PowerUpType>(rand() % numberOfPowerUpTypes);
        float randX = rand() % (1076 - 30);
        float randY = -50;
        genPowerUpClock.restart();
        switch (randomPowerUpType)
        {
        case HEAL:
            powerUpArr.push_back(new HealthRestore(SPACE_TEXTURE, sf::Vector2f(randX, randY)));
            break;
        case SHIELD:
            powerUpArr.push_back(new Shield(SPACE_TEXTURE, sf::Vector2f(randX, randY)));
            break;
        default:
            break;
        }
        // set the direction and speed of the newly added powerup
        powerUpArr.back()->setDirection(sf::Vector2f(0, 1));
        powerUpArr.back()->setSpeed(3);
    }
}

// hmm...
template<class T>
void Game::updateGameObjectArray(std::vector<T*>& arr)
{
    for (auto& v : arr)
        v->move();
}

// hmm...
template<class T>
void Game::drawGameObjectArray(std::vector<T*>& arr)
{
    for (int i = 0; i < arr.size(); i++)
    {
        if (isOutOfBound(arr[i]) == false)
        {
            window->draw(arr[i]->getSprite());
        }
        else
        {
            deleteObjectFromVector(arr, i);
        }
    }
}

void Game::updateGame()
{
    collisionPlayerProjAndEnemy();
    if (player->hasPowerUp(SHIELD))
    {
        collisionEnemyProjAndShield();
        collisionBossProjAndShield();
    }
    else
    {
        collisionEnemyProjAndPlayer();
        collisionEnemyAndPlayer();
    }
    collisionPowerUpAndPlayer();

    tool->update();
    updateGameObjectArray(enemyProjectileArray);
    updateGameObjectArray(playerProjectileArray);
    updateGameObjectArray(enemyArr);
    updateGameObjectArray(powerUpArr);
    generateEnemy();
    generatePowerUp();
    // check all powerup 
    // remove from activePowerUp set 
    // if it passes the duration
    player->removeAllEndedPowerUp();
    boss->updateBossHpBarSize(bossHp);

    //if (tool->getTime().getElapsedTime().asSeconds > 60)
    if (tool->getScore() >= 0)
        BossShown = true;
    // demostration
    enemyRandomShoot();
    if (BossShown) {
        collisionPlayerProjAndBoss();
        collisionBossProjAndPlayer();
        for (auto& proj : bossProjectileArray)
        {
            // only RedSharp will follow the player
            if (proj->getType() == RedSharp) proj->moveToward(*player);
        }
        updateGameObjectArray(bossProjectileArray);
        bossRandomShoot();
    }
    if (InfinityHpTriggered)
    {
        player->setHp(PlanetDefenders::SHIP_MAX_HP[shipNum]);
    }
    tool->updateHpBarSize(player->getHp() / PlanetDefenders::SHIP_MAX_HP[shipNum]);
}

void Game::drawGame() {
    window->draw(GameBackground);
    window->draw(player->getSprite());
    drawGameObjectArray(enemyProjectileArray);
    drawGameObjectArray(playerProjectileArray);
    drawGameObjectArray(enemyArr);
    drawGameObjectArray(powerUpArr);
    drawGameObjectArray(bossProjectileArray);
    // Demo, put to a function later
    // draw shield if player has SHIELD power up
    if (player->hasPowerUp(SHIELD))
    {
        shieldSprite.setPosition(
            player->getSprite().getPosition() +
            sf::Vector2f(
                -9, -7         // offset
            )
        );
        window->draw(shieldSprite);
    }

    window->draw(ToolBarBackground);
    tool->drawTo(*window);
    if (BossShown)
    {
        window->draw(boss->getSprite());
        boss->drawTo(*window);
    }
    window->display();
}

Game::~Game() {
    delete window;
    delete player;
    delete tool;
    playerProjectileArray.clear();
    enemyProjectileArray.clear();
    enemyArr.clear();
    playerProjectileArray.clear();
    powerUpArr.clear();
    bossProjectileArray.clear();
    delete boss;
    delete options;
    delete endscreen;
    delete inputHighscore;
}

void Game::resetGame()
{
    tool->minusScore(tool->getScore());
    tool->restartClock();
    player->setHp(PlanetDefenders::SHIP_MAX_HP[shipNum]);
    backgroundMusic.stop();
    backgroundMusic.play();
    for (int i = 0; i < playerProjectileArray.size(); i++)
    {
        deleteObjectFromVector(playerProjectileArray, i);
    }
    for (int i = 0; i < enemyProjectileArray.size(); i++)
    {
        deleteObjectFromVector(enemyProjectileArray, i);
    }
    playerProjectileArray.clear();
    enemyProjectileArray.clear();
    enemyArr.clear();
    playerProjectileArray.clear();
    powerUpArr.clear();
    bossProjectileArray.clear();
}

void Game::gameLoop() {
    srand(time(0));
    backgroundMusic.setBuffer(titleThemeBuffer);
    backgroundMusic.play();
    // main game loop
    sf::Event e;
    // display menu
    while (!displayMenu()) { sf::sleep(sf::milliseconds(100)); }
    backgroundMusic.setBuffer(backgroundBuffer);
    backgroundMusic.play();
    while (window->isOpen())
    {
        while (window->pollEvent(e))
        {
            switch (e.type)
            {
            case sf::Event::Closed:
                window->close();
                break;
            case sf::Event::KeyReleased:
                if (e.key.code == sf::Keyboard::Key::Escape)
                {
                    GAME_PAUSED ^= 1;
                    pauseGame();
                }
                if (e.key.code == BackdoorTriggerKey)
                    BackdoorTriggered = !BackdoorTriggered;
                if (BackdoorTriggered)
                    handleBackdoorKeyInput(e.key.code);
                break;
            }
        }
        if (!GAME_PAUSED)
        {
            window->clear();
            handleKeyInput();
            updateGame();
            drawGame();
        }
        else
        {
            // prevent drawing TOO MUCH CPU power while pause
            sf::sleep(sf::milliseconds(100));
        }
        // show high score if checkScore is true
        if (player->isDead())
        {
            do {
                if (CheckScore::checkScore(tool->getScore()) == true)
                    inputHighscore->work(*window, *inputHighscore, GameBackground, backgroundMusic, tool->getScore());
            } while (!endscreen->work(*window, *endscreen, GameBackground, backgroundMusic));
            if (window->isOpen() == false) break;       // leave the game
            // reset everything in the game
            resetGame();
            // draw menu again
            backgroundMusic.setBuffer(titleThemeBuffer);
            backgroundMusic.play();
            while (!displayMenu()) { sf::sleep(sf::milliseconds(100)); }
            backgroundMusic.setBuffer(backgroundBuffer);
            backgroundMusic.play();
        }
    }
}
