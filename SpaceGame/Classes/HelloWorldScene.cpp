/**
This file originates from templates from http://www.cocos2d-x.org/
See LICENSE for details of cocos2d-x license.

Portions Copyright (c) 2013 BlackBerry Limited.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
**/

#include "HelloWorldScene.h"

USING_NS_CC;

CCScene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // 'layer' is an autorelease object
    HelloWorld *layer = HelloWorld::create();

    // add layer as a child to scene
    //add a tag so that our AppDelegate can retrieve the scene
    //and call pause/resume methods for application lifecycle
    scene->addChild(layer,1,100);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !CCLayer::init() )
    {
        return false;
    }
    
    CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
    CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();

    
    do {
    	//Initialize game time
    	_gameTime = 0;
    	//Initialize scrolling variables
    	_scrollSpeed = 0;
    	_accelerate = true;
    	_scrollAxis = 0;

		//Get window width/height
		CCSize winSize = CCDirector::sharedDirector()->getWinSize();
		_winHeight = winSize.height;
		_winWidth = winSize.width;

		//Get origin
		_origin = CCDirector::sharedDirector()->getVisibleOrigin();

		//Load spritesheet
		_spriteBatchNode = CCSpriteBatchNode::create("spriteSheet.png");
		this->addChild(_spriteBatchNode);
		CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("spriteSheet.plist");

		//Load background sprites
		_backgroundFirst = CCSprite::createWithSpriteFrameName("background.png");
		_backgroundFirst->setAnchorPoint(ccp(0.0f, 0.0f));
		_backgroundFirst->setPosition(ccp(0.0f, 0.0f));

		CCRect bgRect = _backgroundFirst->getTextureRect();
		_backgroundFirst->setScaleX(winSize.width / bgRect.size.width);
		_backgroundFirst->setScaleY(winSize.height / bgRect.size.height);

		_backgroundSecond = CCSprite::createWithSpriteFrameName("background.png");
		_backgroundSecond->setAnchorPoint(ccp(0.0f, 0.0f));
		_backgroundSecond->setPosition(ccp(0.0f, winSize.height));

		_backgroundSecond->setScaleX(winSize.width / bgRect.size.width);
		_backgroundSecond->setScaleY(winSize.height / bgRect.size.height);
		this->addChild(_backgroundSecond);
		this->addChild(_backgroundFirst);


		//Load Player Spaceship sprite
		_player = CCSprite::createWithSpriteFrameName("playership.png");
		_player->setScaleX(0.2);
		_player->setScaleY(0.2);
		//Position the player sprite at the bottom of the screen
		_player->setPosition(ccp(_winWidth/2,_origin.y + (_player->getScaleY()*_player->getContentSize().height)));
		 this->addChild(_player,10); //Highest Z-order since player sprite needs to be always on the top

		 //Enable Accelerometer
		 this->setAccelerometerEnabled(true);

		 //Preload sound effects
		 CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("shoot.wav");
		 CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("explode.wav");
		 CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("explode_ship.wav");
		 CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadBackgroundMusic("main_background_music.ogg");
		 CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadBackgroundMusic("enemy_background_music.ogg");

		 //Play background music with looping enabled
		 CocosDenshion::SimpleAudioEngine::sharedEngine()->playBackgroundMusic("main_background_music.ogg",true);

		 //Initialize Player projectile count
		 _playerProjectileCount = 0;

		 //Initialize Player mine count
		 _playerMineCount = 0;

		 //Initialize Player projectile sprites
		 for (int i =0; i<MAX_PLAYER_PROJECTILES;++i) {
			 _playerProjectiles[i] =  CCSprite::createWithSpriteFrameName("playerlaser.png");
			 _playerProjectiles[i]->setScaleY(0.2);
			 _playerProjectiles[i]->setVisible(false);
			 _playerProjectiles[i]->setPosition(ccp(_player->getPositionX(),_player->getPositionY()+(_player->getScaleY()*_player->getContentSize().height)));
			 this->addChild(_playerProjectiles[i]);
		 }

		 //Enable touch events
		 this->setTouchEnabled(true);

		 //Initalize current Asteroid count and spawn time
		 _asteroidSpawnTime = 0;
		 _asteroidCount = 0;

		 //Initialize Asteroid sprites
		 for (int i = 0; i < NUM_ASTEROIDS;++i){
		 _asteroids[i] = CCSprite::createWithSpriteFrameName("asteroid.png");
		 _asteroids[i]->setVisible(false);

		 //Position off screen at the top
		 _asteroids[i]->setPosition(ccp(_winWidth,_winHeight+_asteroids[_asteroidCount]->getScaleY()*_asteroids[_asteroidCount]->getContentSize().height));
		 this->addChild(_asteroids[i]);
		 }

		 //Initialize Player health
		 _playerHealth = MAX_PLAYER_HEALTH;

		 //Initilaize Enemy spawn status, current projectile count and health
		 _spawnEnemy = false;
		 _enemyProjectileCount =0;
		 _enemyHealth = MAX_ENEMY_HEALTH;

		 //Initialize Player Mine sprites
		 for (int i =0; i<MAX_PLAYER_MINES;++i) {
			 _playerMines[i] =  CCSprite::createWithSpriteFrameName("playermine.png");
			 _playerMines[i]->setScale(0.2);
			 _playerMines[i]->setVisible(false);
			 _playerMines[i]->setPosition(ccp(_player->getPositionX(),_player->getPositionY()-(_player->getScaleY()*_player->getContentSize().height)));
			 this->addChild(_playerMines[i]);
		 }


		 //Load Enemy Spaceship sprite
		 _enemy = CCSprite::createWithSpriteFrameName("enemyship.png");
		 _enemy->setScaleX(0.3);
		 _enemy->setScaleY(0.3);
		 _enemy->setPosition(ccp(_winWidth/2,_origin.y - (_enemy->getContentSize().height * _enemy->getScaleY())));
		 _enemy->setVisible(false);
		 this->addChild(_enemy);


		 //Initialize Enemy projectile sprites
		 for (int i =0; i < MAX_ENEMY_PROJECTILES;++i) {
		 _enemyProjectiles[i] =  CCSprite::createWithSpriteFrameName("enemylaser.png");
			_enemyProjectiles[i]->setScaleY(0.2);
			_enemyProjectiles[i]->setVisible(false);
			_enemyProjectiles[i]->setPosition(ccp(_enemy->getPositionX(),_enemy->getPositionY()+(_enemy->getScaleY()*_enemy->getContentSize().height)));
			this->addChild(_enemyProjectiles[i]);
		 }

		 //Initialize Player and Enemy health bar sprites
		 _playerHealthBar = CCSprite::createWithSpriteFrameName("health_bar_green.png");
		 _playerHealthBar->setScaleY(0.2);
		 _playerHealthBar->setPosition(ccp(_origin.x+_playerHealthBar->getScaleX()*_playerHealthBar->getContentSize().width/2,_origin.y+_playerHealthBar->getScaleY()*_playerHealthBar->getContentSize().height));
		 this->addChild(_playerHealthBar,10);

		 _playerHealthLabel = CCLabelTTF::create("HEALTH", "Arial", 24);
		 _playerHealthLabel->setPosition(ccp(_origin.x+_playerHealthLabel->getContentSize().width/2,_origin.y+_playerHealthLabel->getContentSize().height+_playerHealthBar->getScaleY()*_playerHealthBar->getContentSize().height));
		 this->addChild(_playerHealthLabel, 10);

		 updatePlayerHealthBar();

		 _enemyHealthBar = CCSprite:: createWithSpriteFrameName ("health_bar_green.png");
		 _enemyHealthBar->setScaleY(0.2);
		 _enemyHealthBar->setPosition(ccp(_origin.x+_enemyHealthBar->getScaleX()*_enemyHealthBar->getContentSize().width/2,_winHeight-_enemyHealthBar->getScaleY()*_enemyHealthBar->getContentSize().height));
		 _enemyHealthBar->setVisible(false);
		 this->addChild(_enemyHealthBar,10);

		 _enemyHealthLabel = CCLabelTTF::create("ENEMY", "Arial", 24);
		 _enemyHealthLabel->setPosition(ccp(_origin.x+_enemyHealthLabel->getContentSize().width/2,_winHeight-_enemyHealthLabel->getContentSize().height-_enemyHealthBar->getScaleY()*_enemyHealthBar->getContentSize().height));
		 _enemyHealthLabel->setVisible(false);
		 this->addChild(_enemyHealthLabel, 10);

		 updateEnemyHealthBar();

		 _gameOver = false;

		 //Initialize game result label
		 _gameResultLabel = CCLabelTTF::create("", "Arial", 60);
		 _gameResultLabel->setVisible(false);
		 this->addChild(_gameResultLabel,1);


		 //Initialize restart menu item
		 _restartMenuItem = CCMenuItemImage::create();
		 _restartMenuItem->setNormalSpriteFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("restart.png"));
		 _restartMenuItem->setTarget(this,menu_selector(HelloWorld::menuRestartCallback));
		 _restartMenuItem->setScale(0.5f);
		 _restartMenuItem->setPosition(_winWidth/2, _winHeight/2 + _restartMenuItem->getScaleY() * _restartMenuItem->getContentSize().height);
		 _restartMenuItem->setEnabled(false);
		 _restartMenuItem->setVisible(false);

		 //Create main menu
		 _mainMenu = CCMenu::create(_restartMenuItem,NULL);
		 _mainMenu->setPosition(CCPointZero);
		 this->addChild(_mainMenu, 1);

		 //Initialize Pause/Resume and toggle menu item
		 _pauseMenuItem = CCMenuItemImage::create();
		 _pauseMenuItem->setNormalSpriteFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("pause.png"));
		 _resumeMenuItem = CCMenuItemImage::create();
		 _resumeMenuItem->setNormalSpriteFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("resume.png"));

		 _pauseResumeToggleItem = CCMenuItemToggle::createWithTarget(this,menu_selector(HelloWorld::menuPauseResumeCallback),_pauseMenuItem,_resumeMenuItem,NULL);
		 _pauseResumeToggleItem->setScale(0.3f);
		 //Position the pause/resume menu item to the top right corner of the screen
		 _pauseResumeToggleItem->setPosition(ccp(visibleSize.width - _pauseResumeToggleItem->getScaleX() * _pauseResumeToggleItem->getContentSize().width/2 ,
													 visibleSize.height- _pauseResumeToggleItem->getScaleY() * _pauseResumeToggleItem->getContentSize().height/2));
		 _mainMenu->addChild(_pauseResumeToggleItem);


		//schedule updateGame to update the game at regular intervals
		this->schedule(schedule_selector(HelloWorld::updateGame));

    } while (0);

    

    return true;
}


void HelloWorld::menuCloseCallback(CCObject* pSender)
{
    CCDirector::sharedDirector()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}


void HelloWorld::updateGame(float dt) {
	_gameTime += dt; //Add dt to game time

	//Background Scrolling
	if (_accelerate) { //accelerate
		//Move the scroll axis at scroll speed
		_scrollAxis -=_scrollSpeed;

		//if we have scrolled through both backgrounds reset scroll axis
		if (_scrollAxis <= -_winHeight)  {
				_scrollAxis = 0;
		}

		//update positions for the background sprites as per the scrollAxis
		_backgroundFirst->setPosition(ccp(0.0f,_scrollAxis));
		_backgroundSecond->setPosition(ccp(0.0f,_scrollAxis+_winHeight));

		//Keep increasing the scroll speed until we approach the max
		if (_scrollSpeed<MAX_SCROLL_SPEED) {
			_scrollSpeed += 0.1;
		}
	} else { //decelerate
		//Move the scroll axis at scroll speed
		_scrollAxis -=_scrollSpeed;

		//if we have scrolled through both backgrounds reset scroll axis
		if (_scrollAxis <= -_winHeight)  {
				_scrollAxis = 0;
		}

		//Keep decreasing the scroll speed until we approach reach 0
		if (_scrollSpeed>0) {
			_scrollSpeed -= 0.1;
			_backgroundFirst->setPosition(ccp(0.0f,_scrollAxis));
			_backgroundSecond->setPosition(ccp(0.0f,_scrollAxis+_winHeight));
		}
	}

	if (!_gameOver) {

		//Update player position based on accelerometer values
		CCPoint location;
		location = ccp(_player->getPositionX()+_aX * ACC_ADJUST_FACTOR,_player->getPositionY());

		//Bound the ship location to size of the screen width so we don't go off the screen
		if (location.x> (_origin.x + _player->getContentSize().width/2 * _player->getScaleX()) && location.x < (_winWidth - _player->getScaleX()*_player->getContentSize().width/2)){
			_player->setPosition(location);

		}

		//Spawn Asteroids
		_asteroidSpawnTime +=dt;
		if (_asteroidSpawnTime > ASTEROID_SPAWN_INTERVAL && _gameTime>ASTEROID_SPAWN_START && _gameTime<ASTEROID_SPAWN_END) {
			_asteroidSpawnTime = 0;

			//Scale the asteroid randomly before spawning it
			float randnum = randomRange(0.1f,0.25f);
			_asteroids[_asteroidCount]->setScaleX(randnum);
			_asteroids[_asteroidCount]->setScaleY(randnum);

			//Spawn at a random X position based on the screen width
			float positionX = randomRange(_asteroids[_asteroidCount]->getContentSize().width/2 * _asteroids[_asteroidCount]->getScaleX(),_winWidth-_asteroids[_asteroidCount]->getContentSize().width/2 * _asteroids[_asteroidCount]->getScaleX());
			_asteroids[_asteroidCount]->stopAllActions();
			_asteroids[_asteroidCount]->setPosition(ccp(positionX,(_winHeight+_asteroids[_asteroidCount]->getScaleY()*_asteroids[_asteroidCount]->getContentSize().height)));
			_asteroids[_asteroidCount]->setVisible(true);

			//Create a random time [2.0,8.0] seconds move action for the asteroid
			_asteroids[_asteroidCount]->runAction(CCSequence::create(CCMoveTo::create(randomRange(2.0f,8.0f), ccp(_asteroids[_asteroidCount]->getPositionX(),_origin.y-_asteroids[_asteroidCount]->getContentSize().height)),CCCallFuncN::create(this,callfuncN_selector(HelloWorld::spriteMoveFinished)),NULL));
			++_asteroidCount;

			 //If have used up all available sprites reset the the count to reuse the sprites
			 if (_asteroidCount>=NUM_ASTEROIDS) {
				 _asteroidCount = 0;
			 }
		}

		//Spawn Enemy 5 seconds after spawning the last asteroid
		if (_gameTime>ASTEROID_SPAWN_END+5.0f && !_spawnEnemy) {
			//Stop backgrond scrolling
			//_accelerate = false;
			CocosDenshion::SimpleAudioEngine::sharedEngine()->playBackgroundMusic("enemy_background_music.ogg",true);
			_enemy->setVisible(true);

			//Reposition Player
			CCMoveTo *playerMove = CCMoveTo::create(1.5, ccp(_winWidth/2, _winHeight/2+_player->getContentSize().height*_player->getScaleY()));
			_player->runAction(CCSequence::create(playerMove,NULL));


			//Bring in the Enemy with a move action
			CCMoveTo *enemyMove = CCMoveTo::create(1.5, ccp(_winWidth/2, _origin.y+_enemy->getContentSize().height*_enemy->getScaleY()/2));
			_enemy->runAction(CCSequence::create(enemyMove,CCCallFuncN::create(this,callfuncN_selector(HelloWorld::enemyMoveFinished)),NULL));
			_spawnEnemy = true;
			_enemyHealthBar->setVisible(true);
			_enemyHealthLabel->setVisible(true);
		}

		//Collision detection Asteroid<->Player, Asteroid<->Player projectiles
		for (int i = 0; i < NUM_ASTEROIDS; ++i) {
			//We only need to check collisions if the current Asteroid and Player are visible
			if (_asteroids[i]->isVisible() && _player->isVisible()) {
				if (_asteroids[i]->boundingBox().intersectsRect(_player->boundingBox())) {
					--_playerHealth;
					updatePlayerHealthBar();
					CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("explode.wav");
					if (_playerHealth <=0) {
						_player->stopAllActions();
						_player->setVisible(false);
						//Destroy Player spaceship with a sound effect and a custom exploding ring particle effect
						ccColor4F startColor = {0.4f, 0.5f, 1.0f, 1.0f};
						createParticleEffect("Particles/ExplodingRing.plist",_player->getPositionX(),_player->getPositionY(),startColor,1.0f,100.0f);
						CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("explode_ship.wav");
						endGame(false);
					} else {
						_asteroids[i]->setVisible(false);
						//Create an exploding ring particle effect for the Asteroid destruction
						createParticleEffect("Particles/ExplodingRing.plist",_asteroids[i]->getPositionX(),_asteroids[i]->getPositionY());
						//Create a blinking effect signifying player ship damage
						CCBlink *blinkAction = CCBlink::create(1.0f,10);
						CCShow *showAction = CCShow::create();
						CCSequence *action = CCSequence::create(blinkAction,showAction,NULL);
						_player->runAction(action);
					}
				}

				//For each Player projectile check for collision with the current asteroid
				for (int j = 0; j < MAX_PLAYER_PROJECTILES; ++j) {
					if (_playerProjectiles[j]->isVisible()) {
						if (_playerProjectiles[j]->boundingBox().intersectsRect(_asteroids[i]->boundingBox())) {
							_asteroids[i]->setVisible(false);
							_playerProjectiles[j]->setVisible(false);
							createParticleEffect("Particles/ExplodingRing.plist",_asteroids[i]->getPositionX(),_asteroids[i]->getPositionY());
							CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("explode.wav");
						}
					}
				}
			}
		}

		//Collision detection Player Mines <-> Enemy
		for (int j = 0; j < MAX_PLAYER_MINES; ++j) {
			//We only need to check collisions if the current Player projectile and Enemy are visible
			if (_playerMines[j]->isVisible() && _enemy->isVisible()) {
				if (_playerMines[j]->boundingBox().intersectsRect(_enemy->boundingBox())) {
					_playerMines[j]->setVisible(false);
					//Create an exploding ring particle effect for the Mine Explosion
					createParticleEffect("Particles/ExplodingRing.plist",_playerMines[j]->getPositionX(),_playerMines[j]->getPositionY());
					CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("explode.wav");
					--_enemyHealth;
					updateEnemyHealthBar();
					//Create Small Sun particle effect to signify Enemy damage
					createParticleEffect("Particles/SmallSun.plist",_enemy->getPositionX(),_enemy->getPositionY());
					if (_enemyHealth <= 0) {
						//Destroy Enemy spaceship with a sound effect and a custom galaxy particle effect
						_enemy->setVisible(false);;
						ccColor4F startColor = {1.0f, 0.5f, 0.5f, 1.0f};
						createParticleEffect("Particles/Galaxy.plist",_enemy->getPositionX(),_enemy->getPositionY(),startColor,2.0f,200.0f);
						CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("explode_ship.wav");
						endGame(true);
					}

				}
			}
		}

		//Collision detection Enemy Projectiles <-> Player
		for (int j = 0; j < MAX_ENEMY_PROJECTILES; ++j) {

			//We only need to check collisions if the current Enemy projectile and Player are visible
			if (_enemyProjectiles[j]->isVisible() && _player->isVisible()) {
				if (_enemyProjectiles[j]->boundingBox().intersectsRect(_player->boundingBox())) {
					_enemyProjectiles[j]->setVisible(false);
					--_playerHealth;
					updatePlayerHealthBar();
					//Create Small Sun particle effect to signify Player damage
					createParticleEffect("Particles/SmallSun.plist",_player->getPositionX(),_player->getPositionY());
					CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("explode.wav");
					if (_playerHealth <=0) {
						_player->stopAllActions();
						_player->setVisible(false);
						//Destroy Player spaceship with a sound effect and a custom exploding ring particle effect
						ccColor4F startColor = {0.4f, 0.5f, 1.0f, 1.0f};
						createParticleEffect("Particles/ExplodingRing.plist",_player->getPositionX(),_player->getPositionY(),startColor,1.0f,100.0f);
						CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("explode_ship.wav");
						endGame(false);
					} else {
						//Create a blinking effect signifying Player spaceship damage
						CCBlink *blinkAction = CCBlink::create(1.0f,10);
						CCShow *showAction = CCShow::create();
						CCSequence *action = CCSequence::create(blinkAction,showAction,NULL);
						_player->runAction(action);
					}
				}
			}
		}

	}

}

void HelloWorld::didAccelerate(CCAcceleration *pAccelerationValue) {
	_aX = pAccelerationValue->x;
	_aY = pAccelerationValue->y;
	_aZ = pAccelerationValue->z;

}

void HelloWorld::ccTouchesBegan(cocos2d::CCSet *touches, cocos2d::CCEvent *event ) {
	if (!_gameOver && !_gamePaused) {
		//Choose one of the touches to work with
		CCTouch* touch = (CCTouch*)( touches->anyObject());
		CCPoint location = touch->getLocation();
		if (!_spawnEnemy) {
			//Pick laser sprite from the projectile array and run an action on it
			_playerProjectiles[_playerProjectileCount]->stopAllActions();
			_playerProjectiles[_playerProjectileCount]->setPosition(ccp(_player->getPositionX(),_player->getPositionY()+(_player->getScaleY()*_player->getContentSize().height)));
			_playerProjectiles[_playerProjectileCount]->setVisible(true);

			//Create a move action that is 0.4s long and moves the projectile starting from the player position to the top of the screen
			_playerProjectiles[_playerProjectileCount]->runAction(CCSequence::create(CCMoveTo::create(0.4f, ccp(_player->getPositionX(),_winHeight)),CCCallFuncN::create(this,callfuncN_selector(HelloWorld::spriteMoveFinished)),NULL));
			++_playerProjectileCount;

			//If reached the maximum number of sprites reset the the count to recycle the sprites
			if (_playerProjectileCount>=MAX_PLAYER_PROJECTILES) {
				_playerProjectileCount = 0;
			}
			CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("shoot.wav");
		} else {
			//Pick laser sprite from the projectile array and run an action on it
			_playerMines[_playerMineCount]->stopAllActions();
			_playerMines[_playerMineCount]->setPosition(ccp(_player->getPositionX(),_player->getPositionY()-(_player->getScaleY()*_player->getContentSize().height)));
			_playerMines[_playerMineCount]->setVisible(true);

			_playerMines[_playerMineCount]->runAction(CCRepeatForever::create(CCRotateBy::create(1.0f, 360.0f)));
			//Create a move action that is 2.0s long and moves the mine starting from the player position to the bottom of the screen
			_playerMines[_playerMineCount]->runAction(CCSequence::create(CCMoveTo::create(2.0f, ccp(_player->getPositionX(),_origin.x)),CCCallFuncN::create(this,callfuncN_selector(HelloWorld::spriteMoveFinished)),NULL));
			++_playerMineCount;

			//If reached the maximum number of sprites reset the the count to recycle the sprites
			if (_playerMineCount>=MAX_PLAYER_MINES) {
				_playerMineCount = 0;
			}
			//CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("shoot.wav");
		}
	}
}

void HelloWorld::spriteMoveFinished(CCNode* sender) {
	sender->setVisible(false);
}

float HelloWorld::randomRange(float min, float max) {
	float randnum = (float) rand() / (float)RAND_MAX;
	return min + randnum * (max - min);
}

void HelloWorld::createParticleEffect(const char* filename, float x, float y) {
	 CCParticleSystem* emitter = CCParticleSystemQuad::create(filename);
	 emitter->setPosition(x,y);
	 emitter->setAutoRemoveOnFinish(true);
	 addChild(emitter, 10);
}

void HelloWorld::createParticleEffect(const char* filename, float x, float y, ccColor4F startColor, float duration, float endSize) {
	 CCParticleSystem* emitter = CCParticleSystemQuad::create(filename);
	 emitter->setPosition(x,y);
	 emitter->setStartColor(startColor);
	 emitter->setDuration(duration);
	 emitter->setEndSize(endSize);
	 emitter->setAutoRemoveOnFinish(true);
	 addChild(emitter, 10);
}

void HelloWorld::enemyMoveFinished(CCNode* sender) {
	//If Enemy is not destroyed keep animating
	if (_enemyHealth>0) {
		enemyAnimate();
	}
}

void HelloWorld::enemyAnimate() {
	//Set the Enemy move postion to current Enemy Y and Player X
	CCPoint enemyPosition = ccp(_player->getPositionX(),_enemy->getPositionY());

	//If the new Enemy position is outside the visible width of the screen
	//Set the position to max left or max right
	if (enemyPosition.x + _enemy->getContentSize().width/2 * _enemy->getScaleX() > _winWidth) {
		enemyPosition.setPoint(_winWidth - _enemy->getScaleX() * _enemy->getContentSize().width/2,enemyPosition.y);
	} else if (enemyPosition.x - _enemy->getContentSize().width/2 * _enemy->getScaleX() < _origin.x) {
    	enemyPosition.setPoint(_origin.x + _enemy->getScaleX() * _enemy->getContentSize().width/2,enemyPosition.y);
	}

	_enemy->runAction(CCSequence::create(CCMoveTo::create(0.4f,enemyPosition),CCCallFuncN::create(this,callfuncN_selector(HelloWorld::enemyMoveFinished)),NULL));

	//Shooting Delay
	float delay = 1.0f;

	//Distance between the Enemy laser projectiles relative to the Enemy sprite width
	float widthAdjustment = 4.0f;

	//Launch four projectiles with two sets of delays and X-coordinates based on the Enemy Spaceship width
	for (int i = 0; i < 2; ++i) {
		_enemyProjectiles[_enemyProjectileCount]->stopAllActions();
		_enemyProjectiles[_enemyProjectileCount]->setPosition(ccp(_enemy->getPositionX()-_enemy->getScaleX()*_enemy->getContentSize().width/widthAdjustment,_enemy->getPositionY()));
		_enemyProjectiles[_enemyProjectileCount]->setVisible(true);
		_enemyProjectiles[_enemyProjectileCount]->runAction(CCSequence::create(CCMoveTo::create(delay, ccp(_enemy->getPositionX()-_enemy->getScaleX()*_enemy->getContentSize().width/widthAdjustment,_winHeight)),CCCallFuncN::create(this,callfuncN_selector(HelloWorld::spriteMoveFinished)),NULL));

		++_enemyProjectileCount;

		_enemyProjectiles[_enemyProjectileCount]->stopAllActions();
		_enemyProjectiles[_enemyProjectileCount]->setPosition(ccp(_enemy->getPositionX()+_enemy->getScaleX()*_enemy->getContentSize().width/widthAdjustment,_enemy->getPositionY()));
		_enemyProjectiles[_enemyProjectileCount]->setVisible(true);
		_enemyProjectiles[_enemyProjectileCount]->runAction(CCSequence::create(CCMoveTo::create(delay, ccp(_enemy->getPositionX()+_enemy->getScaleX()*_enemy->getContentSize().width/widthAdjustment,_winHeight)),CCCallFuncN::create(this,callfuncN_selector(HelloWorld::spriteMoveFinished)),NULL));

		++_enemyProjectileCount;

		//For next two projectiles
		//Double the delay to make inner lasers moving slower
		delay += 0.5f;
		//Make the width closer to the center of the Enemy spaceship
		widthAdjustment *= 3.0f;
	}

	//If reached the maximum number of sprites reset the the count to recycle the sprites
	if (_enemyProjectileCount>=MAX_ENEMY_PROJECTILES) {
		_enemyProjectileCount = 0;
	}
}

void HelloWorld::updatePlayerHealthBar() {
	//Scale the health bar by dividing current health by max health
	float scaleFactor = ((float)_playerHealth / MAX_PLAYER_HEALTH);
 	_playerHealthBar->setScaleX(scaleFactor);
	_playerHealthBar->setPosition(ccp(_origin.x+_playerHealthBar->getScaleX()*_playerHealthBar->getContentSize().width/2,_origin.y+_playerHealthBar->getScaleY()*_playerHealthBar->getContentSize().height));
	//If health is between 40-60% display yellow bar
	if (scaleFactor*10 <=6 && scaleFactor*10 >= 4) {
		_playerHealthBar->setDisplayFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("health_bar_yellow.png"));
	} else if (scaleFactor*10 < 4) { //If health is less than 40% display blinking red bar
		_playerHealthBar->setDisplayFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("health_bar_red.png"));

		//Create a blinking effect indicating low health
		 CCBlink *blinkAction = CCBlink::create(1.0f,4);
		 CCShow *showAction = CCShow::create();
		 CCSequence *action = CCSequence::create(blinkAction,showAction,NULL);
		 CCRepeatForever *repeatAction = CCRepeatForever::create(action);
		_playerHealthBar->runAction(repeatAction);
	}

}


void HelloWorld::updateEnemyHealthBar() {
	//Scale the health bar by dividing current health by max health
	float scaleFactor = ((float)_enemyHealth / MAX_ENEMY_HEALTH);
	_enemyHealthBar->setScaleX(scaleFactor);
	_enemyHealthBar->setPosition(ccp(_origin.x+_enemyHealthBar->getScaleX()*_enemyHealthBar->getContentSize().width/2,_winHeight-_enemyHealthBar->getScaleY()*_enemyHealthBar->getContentSize().height));
	 //If health is between 40-60% display yellow bar
	if (scaleFactor*10 <=6 && scaleFactor*10 >= 4) {
		_enemyHealthBar->setDisplayFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("health_bar_yellow.png"));
	} else if (scaleFactor*10 < 4) {
		_enemyHealthBar->setDisplayFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("health_bar_red.png"));

		//Create a blinking effect indicating low health
		 CCBlink *blinkAction = CCBlink::create(1.0f,4);
		 CCShow *showAction = CCShow::create();
		 CCSequence *action = CCSequence::create(blinkAction,showAction,NULL);
		 CCRepeatForever *repeatAction = CCRepeatForever::create(action);
		 _enemyHealthBar->runAction(repeatAction);
	}
}

void HelloWorld::menuRestartCallback(CCObject* sender) {

	//if the game is paused we need to resume the game first before resetting otherwise we will end up
	//restarting in paused state
	if (_gamePaused) {
			resumeGame();
		}

	//Restart game with a cool rotozoom transition
	CCDirector::sharedDirector()->replaceScene(CCTransitionRotoZoom::create(1.0, this->scene()));
}

void HelloWorld::endGame(bool won) {
	if (won) {
		_enemyHealthLabel->setVisible(false);
		_enemyHealthBar->setVisible(false);
		_gameResultLabel->setString("YOU WIN!");

	} else {
		_playerHealthLabel->setVisible(false);
		_playerHealthBar->setVisible(false);
		_gameResultLabel->setString("GAME OVER");
	}
	_gameResultLabel->setPosition(ccp(_winWidth/2,_winHeight/2 -_gameResultLabel->getContentSize().height));
	_gameResultLabel->setVisible(true);
	_gameOver = true;
	_restartMenuItem->setVisible(true);
	_restartMenuItem->setEnabled(true);
}

void HelloWorld::pauseGame() {
	//Pause scene
	CCDirector::sharedDirector()->pause();
	_gamePaused = true;
	//Pause audio playback
	CocosDenshion::SimpleAudioEngine::sharedEngine()->pauseAllEffects();
	CocosDenshion::SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();
	//When the AppDelegate directly pauses the game we want to ensure the resume item is displayed
	_pauseResumeToggleItem->setSelectedIndex(1);

}

void HelloWorld::resumeGame() {
	//Resume scene
	CCDirector::sharedDirector()->resume();
	_gamePaused = false;
	//Resume audio playback
	CocosDenshion::SimpleAudioEngine::sharedEngine()->resumeAllEffects();
	CocosDenshion::SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic();
}

void HelloWorld::menuPauseResumeCallback(CCObject* sender) {
	  CCMenuItemToggle *toggleItem = (CCMenuItemToggle *)sender;
	  if (toggleItem->selectedItem() == _resumeMenuItem) {
		  pauseGame();
	  } else if (toggleItem->selectedItem() == _pauseMenuItem) {
		  resumeGame();
	  }
}






