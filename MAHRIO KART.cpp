// Niall Starkey - MAHRIO KART.cpp

//////////////////
//   HEADERS:   //
//////////////////
#include <TL-Engine.h>
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <vector>
#include <math.h>
#include "xcontroller.h"

using namespace tle;

//////////////////
//   STRUCTS:   //
//////////////////
struct Vector2D
{
	float x;
	float z;
};

////////////////
//   ENUMS:   //
////////////////
enum GameState {Paused, Racing};
enum ControlType { Keyboard, Controller };
enum Camera { FirstPerson, ThirdPerson };
enum Section {StageOne, StageTwo, StageThree, StageFour, StageFive, Complete};
enum Direction {Forward, Backward, Left, Right, Diagonal, None};

///////////////////////////////
//   FORWARD DECLARATIONS:   //
///////////////////////////////
Direction CalculateBounceDirection(IModel* player, IModel* object, Vector2D momentum, float objectLength, float objectWidth);
Vector2D Scalar(float scale, Vector2D vector);
Vector2D VectorAddition(Vector2D vector1, Vector2D vector2, Vector2D vector3);
void ReadTrackFile(string fileLocation);
void CalculateBounce(IModel* player, Direction direction, Vector2D momentum);
bool PointToBox(IModel* point, IModel* box, int length, int width);
bool SphereToBox(IModel* sphere, IModel* box, int radius, int length, int width);
bool SphereToSphere(IModel* sphere, IModel* sphere2, int radius, int radius2);
int RandomGenerator(int minumum, int maximum);


////////////////////
//   CONSTANTS:   //
////////////////////
const float kDeadZone = 0.2f;
const float kGameScale = 5.0f;
const float kStartHealth = 100.0f;
const float kRotateSpeed = 100.0f;
const float kThrustFactor = 0.1f;
const float kReverseFactor = 0.05f;
const float kDragCo = -0.001f;
const float kShipRadius = 3.0f;

//   KEYBOARD KEY CODES:   //
const EKeyCode escape = Key_Escape;
const EKeyCode space = Key_Space;
const EKeyCode up = Key_Up;
const EKeyCode down = Key_Down;
const EKeyCode leftKey = Key_Left;
const EKeyCode rightKey = Key_Right;
const EKeyCode wKey = Key_W;
const EKeyCode aKey = Key_A;
const EKeyCode sKey = Key_S;
const EKeyCode dKey = Key_D;
const EKeyCode one = Key_1;
const EKeyCode two = Key_2;
const EKeyCode equals = Key_Minus;

//   CONTROLLER BUTTON CODES:   //
const EButtonCode start = Button_Start;
const EButtonCode select = Button_Back;
const EButtonCode shoulderR = Button_R;
const EButtonCode shoulderL = Button_L;
const EButtonCode thumbRPress = Button_RThumb;
const EButtonCode thumbLPress = Button_LThumb;

//   CONTROLLER ANALOGUE CODES:   //
const EAnalogueCode triggerR = Analogue_RTrigger;
const EAnalogueCode triggerL = Analogue_LTrigger;
const EAnalogueCode thumbL = Analogue_LStickX;
const EAnalogueCode thumbRX = Analogue_RStickX;
const EAnalogueCode thumbRY = Analogue_RStickY;

//////////////////
//   VECTORS:   //
//////////////////
vector<IMesh*> meshes;
vector<IModel*> checkpoints;
vector<IModel*> isles;
vector<IModel*> walls;
vector<IModel*> tanks;
vector<IModel*> crosses;
vector<IModel*> checkpointDummies;
vector<IModel*> tankDummies;
vector<IModel*> waypoints;

///////////////dd
//   MAIN:   //
///////////////
void main()
{
	//   SETUP:   //
	I3DEngine* engine = New3DEngine( kTLX );
	engine->StartWindowed();

	IFont* font = engine->LoadFont("Comic Sans MS", 20);

	engine->AddMediaFolder("./Media");

	XController* controller = new XController();

	//   CONSTANTS:   //
	const string kTrackLocation = "./Positions/Main.txt";
	const string kShip = "race2.x";
	const string kEnemy = "enemy.x";
	const string kGround = "ground.x";
	const string kSky = "skybox 07.x";
	const string kCheckpoint = "checkpoint.x";
	const string kIsle = "islestraight.x";
	const string kWall = "wall.x";
	const string kWaterTank = "tanksmall1.x";
	const string kWaterTank2 = "tanksmall2.x";
	const string kUI = "ui_backdrop.jpg";
	const string kCross = "cross.x";
	const string kDummy = "dummy.x";
	const string kEnemySkin = "sp02.jpg";

	//   VARIABLES:  //
	ControlType currentControl = Keyboard;
	Camera currentCameraLocation = ThirdPerson;
	GameState currentState = Paused;
	Section currentSection = StageOne;
	Direction currentBounce = Forward;
	Vector2D momentum = { 0.0f, 0.0f };
	Vector2D enemyMomentum = { 0.0f, 0.0f };
	Vector2D thrust = { 0.0f, 0.0f };
	Vector2D enemyThrust = { 0.0f, 0.0f };
	Vector2D drag = { 0.0f, 0.0f };
	Vector2D enemyDrag = { 0.0f, 0.0f };

	bool fileLoaded = false;
	bool countdown = false;
	bool overheat = false;
	bool boostDisabled = false;
	float playerMatrix[4][4];
	float enemyMatrix[4][4];
	float count = 4;
	float fpsRefresh = 0;
	float speedRefresh = 0;
	float boostTime = 0;
	float cooldownTime = 0;
	float frameTime;
	float FPS = 0;
	float kmph = 0;
	float currentHealth = kStartHealth;

	ISprite* backdrop = engine->CreateSprite(kUI, 0, 660);

	//   MESHES:   //
	IMesh* playerMesh = engine->LoadMesh(kShip);
	meshes.push_back(playerMesh);
	IMesh* skyMesh = engine->LoadMesh(kSky);
	meshes.push_back(skyMesh);
	IMesh* groundMesh = engine->LoadMesh(kGround);
	meshes.push_back(groundMesh);
	IMesh* checkpointMesh = engine->LoadMesh(kCheckpoint);
	meshes.push_back(checkpointMesh);
	IMesh* isleMesh = engine->LoadMesh(kIsle);
	meshes.push_back(isleMesh);
	IMesh* wallMesh = engine->LoadMesh(kWall);
	meshes.push_back(wallMesh);
	IMesh* waterMesh = engine->LoadMesh(kWaterTank);
	meshes.push_back(waterMesh);
	IMesh* waterMesh2 = engine->LoadMesh(kWaterTank2);
	meshes.push_back(waterMesh2);
	IMesh* crossMesh = engine->LoadMesh(kCross);
	meshes.push_back(crossMesh);
	IMesh* dummyMesh = engine->LoadMesh(kDummy);
	meshes.push_back(dummyMesh);
	IMesh* enemyMesh = engine->LoadMesh(kEnemy);
	meshes.push_back(enemyMesh);

	//   MODELS:   //
	IModel* player = playerMesh->CreateModel(5, 0, -50);
	IModel* enemy = enemyMesh->CreateModel(-10, 0, -40);
	IModel* sky = skyMesh->CreateModel(0, -960, 0);
	IModel* ground = groundMesh->CreateModel();

	//   CAMERA:   //
	ICamera* camera = engine->CreateCamera(kManual);
	camera->SetPosition(0, 10, -30);
	camera->AttachToParent(enemy);

	//   MODEL INITIALISATION:   //
	ReadTrackFile(kTrackLocation);

	engine->Timer();

	////////////////////
	//   GAME LOOP:   //
	////////////////////
	while (engine->IsRunning())
	{
		//   SCENE DRAW:   //
		engine->DrawScene();
		frameTime = engine->Timer();
		controller->UpdateInput();

		//   COUNTERS:   //
		if (fpsRefresh > 0.5)
		{
			FPS = 1 / frameTime;
			fpsRefresh = 0;
		}
		else
		{
			fpsRefresh += frameTime;
		}

		if (speedRefresh > 0.5)
		{
			float distanceZ = momentum.z * momentum.z;
			kmph = kGameScale * FPS * distanceZ;
		}
		else
		{
			speedRefresh += frameTime;
		}

		if (countdown)
		{
			count -= frameTime;
		}

		if (overheat)
		{
			cooldownTime += frameTime;
		}

		//   UI OUTPUT:   //
		stringstream speed;
		stringstream time;
		stringstream state;
		stringstream counter;
		stringstream health;
		stringstream boostActive;
		stringstream overheatWarning;
		stringstream cooldown;
		stringstream timer;

		speed << "Speed = " << trunc(kmph) << "km/h";
		time << "FPS = " << trunc(FPS);
		counter << "COUNT = " << count;
		health << "HEALTH = " << currentHealth;
		boostActive << "BOOSTING";
		overheatWarning << "OVERHEATING";
		cooldown << "COOLING DOWN";
		timer << "COOLDOWN = " << cooldownTime;

		font->Draw(speed.str(), 10, 690);
		font->Draw(time.str(), 1270, 690, kGreen, kRight);
		font->Draw(counter.str(), 10, 10);
		font->Draw(health.str(), 10, 670, kRed);
		font->Draw(timer.str(), 30, 30);

		if (boostTime > 2 && boostTime < 3)
		{
			font->Draw(overheatWarning.str(), 640, 600, kRed, kCentre, kVCentre);
		}

		if (currentState == Paused)
		{
			state << "PAUSED";
			font->Draw(state.str(), 640, 700, kBlack, kCentre, kVCentre);
		}
		else if (currentState == Racing && currentSection == StageOne)
		{
			state << "STAGE ONE";
			font->Draw(state.str(), 640, 700, kBlack, kCentre, kVCentre);
		}
		else if (currentState == Racing && currentSection == StageTwo)
		{
			state << "STAGE TWO";
			font->Draw(state.str(), 640, 700, kBlack, kCentre, kVCentre);
		}
		else if (currentState == Racing && currentSection == StageThree)
		{
			state << "STAGE THREE";
			font->Draw(state.str(), 640, 700, kBlack, kCentre, kVCentre);
		}
		else if (currentState == Racing && currentSection == StageFour)
		{
			state << "STAGE FOUR";
			font->Draw(state.str(), 640, 700, kBlack, kCentre, kVCentre);
		}
		else if (currentState == Racing && currentSection == StageFive)
		{
			state << "STAGE FIVE";
			font->Draw(state.str(), 640, 700, kBlack, kCentre, kVCentre);
		}
		else if (currentState == Racing && currentSection == Complete)
		{
			state << "RACE COMPLETE";
			font->Draw(state.str(), 640, 700, kBlack, kCentre, kVCentre);
		}

		//   PAUSED:   //
		if (currentState == Paused)
		{
			stringstream dialog;
			if (!countdown)
			{
				dialog << "HIT SPACE TO START";
				count = 4;
				font->Draw(dialog.str(), 640, 675, kRed, kCentre, kVCentre);
			}

			if (countdown)
			{
				if (count < 4.0f && count > 3.0f)
				{
					dialog << "3";
				}
				else if (count < 3.0f && count > 2.0f)
				{
					dialog << "2";
				}
				else if (count < 2.0f && count > 1.0f)
				{
					dialog << "1";
				}
				else if (count < 1.0f)
				{
					currentState = Racing;
				}
				font->Draw(dialog.str(), 640, 675, kRed, kCentre, kVCentre);
			}

			if (engine->KeyHit(space))
			{
				countdown = true;
			}
		}
		//   RACING:   //
		else if (currentState == Racing)
		{
			if (count < 1.0f && count > 0.0f)
			{
				stringstream dialog;
				dialog << "GO";
				font->Draw(dialog.str(), 640, 675, kRed, kCentre, kVCentre);
			}

			float mouseX = engine->GetMouseMovementX();
			float mouseY = engine->GetMouseMovementY();
			float lStickX = controller->GetAnalogueInput(thumbL);
			float rStickX = controller->GetAnalogueInput(thumbRX);
			float rStickY = controller->GetAnalogueInput(thumbRY);
			float lTrigger = controller->GetAnalogueInput(triggerL);
			float rTrigger = controller->GetAnalogueInput(triggerR);

			camera->RotateY(mouseX);

			player->GetMatrix(&playerMatrix[0][0]);
			Vector2D facingVector{ playerMatrix[2][0], playerMatrix[2][2] };
			Vector2D reverseVector{ -playerMatrix[2][0], -playerMatrix[2][2] };

			enemy->GetMatrix(&enemyMatrix[0][0]);
			Vector2D enemyFacingVector{ enemyMatrix[2][0], enemyMatrix[2][2] };

			//   CONTROLLER:   //
			if (currentControl == Controller)
			{
				if (fabs(lStickX) > kDeadZone)
				{
					player->RotateY((kRotateSpeed * lStickX) * frameTime);
				}

				if (fabs(rStickX) > kDeadZone)
				{
					camera->RotateY((0.5f * rStickX) * frameTime);
				}

				if (fabs(rStickY) > kDeadZone)
				{
					camera->MoveLocalZ((0.1f * rStickY) * frameTime);
				}

				if (rTrigger > kDeadZone)
				{
					thrust = Scalar(((kThrustFactor * rTrigger) * frameTime), facingVector);
				}

				if (lTrigger > kDeadZone)
				{
					thrust = Scalar(-((kReverseFactor * lTrigger) * frameTime), facingVector);
				}

				if (lTrigger < kDeadZone && rTrigger < kDeadZone)
				{
					thrust = { 0.0f, 0.0f };
				}

				if (controller->ButtonHit(thumbRPress))
				{
					camera->ResetOrientation();
				}

				if (controller->ButtonHit(thumbLPress))
				{
					currentControl = Keyboard;
				}
			}
			//   KEYBOARD:   //
			else if (currentControl == Keyboard)
			{
				if (currentHealth > 0)
				{
					if (engine->KeyHeld(aKey))
					{
						player->RotateY((-kRotateSpeed * frameTime));
					}

					if (engine->KeyHeld(dKey))
					{
						player->RotateY((kRotateSpeed * frameTime));
					}

					if (engine->KeyHeld(wKey))
					{
						if (engine->KeyHeld(space) && currentHealth > 30 && !boostDisabled)
						{
							thrust = Scalar(((kThrustFactor * 1.5) * frameTime), facingVector);
						}
						else
						{
							thrust = Scalar((kThrustFactor * frameTime), facingVector);
						}		
					}
					else if (engine->KeyHeld(sKey))
					{
						if (engine->KeyHeld(space) && currentHealth > 30 && !boostDisabled)
						{
							thrust = Scalar(-((kReverseFactor * 1.5) * frameTime), facingVector);
						}
						else
						{
							thrust = Scalar(-(kReverseFactor * frameTime), facingVector);
						}
					}
					else
					{
						thrust = { 0.0f, 0.0f };
					}
				}

				if (engine->KeyHeld(up))
				{
					camera->MoveLocalZ((kRotateSpeed * frameTime));
				}

				if (engine->KeyHeld(down))
				{
					camera->MoveLocalZ(-(kRotateSpeed * frameTime));
				}

				if (engine->KeyHeld(space) && boostDisabled == false)
				{
					font->Draw(boostActive.str(), 640, 650, kRed, kCentre, kVCentre);
					boostTime += frameTime;

					if (boostTime > 3)
					{
						boostTime = 0;
						overheat = true;
						cooldownTime = 0;
						boostDisabled = true;
					}
				}
				else
				{
					boostTime = 0;
				}

				if (engine->KeyHeld(leftKey))
				{
					camera->MoveLocalX(-(kRotateSpeed * frameTime));
				}

				if (engine->KeyHeld(rightKey))
				{
					camera->MoveLocalX((kRotateSpeed * frameTime));
				}

				if (engine->KeyHit(equals))
				{
					currentControl = Controller;
				}
			}
			
			if (boostDisabled)
			{
				cooldownTime += frameTime;
				font->Draw(cooldown.str(), 640, 650, kRed, kCentre, kVCentre);
			}

			if (cooldownTime > 5)
			{
				cooldownTime = 0;
				boostDisabled = false;
			}

			//   CAR MOVEMENT:   //
			if (cooldownTime < 5 && boostDisabled)
			{
				drag = Scalar(((kDragCo) * 2), momentum);
			}
			else
			{
				drag = Scalar((kDragCo), momentum);
			}
			momentum = VectorAddition(momentum, thrust, drag);
			player->Move(momentum.x, 0.0f, momentum.z);

			if (SphereToSphere(enemy, waypoints[0], 3, 5))
			{
				waypoints.push_back(waypoints[0]);
				waypoints.erase(waypoints.begin());
			}
			enemy->LookAt(waypoints[0]);
			if (enemy->GetX() != waypoints[0]->GetX() && enemy->GetZ() != waypoints[0]->GetZ())
			{
				enemyThrust = Scalar((kThrustFactor * frameTime), enemyFacingVector);
			}
			enemyDrag = Scalar((kDragCo), enemyMomentum);
			enemyMomentum = VectorAddition(enemyMomentum, enemyThrust, enemyDrag);
			enemy->Move(enemyMomentum.x, 0.0f, enemyMomentum.z);

			//   CHECKPOINTS PASS:   //
			for (int i = 0; i < checkpoints.size(); i++)
			{
				if (PointToBox(player, checkpoints[i], 8, 1))
				{
					if (currentSection == StageOne && i == 0)
					{
						IModel* cross = crossMesh->CreateModel(checkpoints[i]->GetX(), 5, checkpoints[i]->GetZ());
						cross->Scale(0.5);
						crosses.push_back(cross);
						currentSection = StageTwo;
					}
					else if (currentSection == StageTwo && i == 1)
					{
						IModel* cross = crossMesh->CreateModel(checkpoints[i]->GetX(), 5, checkpoints[i]->GetZ());
						cross->Scale(0.5);
						cross->RotateY(90);
						crosses.push_back(cross);
						currentSection = StageThree;
					}
					else if (currentSection == StageThree && i == 2)
					{
						IModel* cross = crossMesh->CreateModel(checkpoints[i]->GetX(), 5, checkpoints[i]->GetZ());
						cross->Scale(0.5);
						crosses.push_back(cross);
						currentSection = StageFour;
					}
					else if (currentSection == StageFour && i == 3)
					{
						IModel* cross = crossMesh->CreateModel(checkpoints[i]->GetX(), 5, checkpoints[i]->GetZ());
						cross->Scale(0.5);
						crosses.push_back(cross);
						currentSection = StageFive;
					}
					else if (currentSection == StageFive && i == 4)
					{
						IModel* cross = crossMesh->CreateModel(checkpoints[i]->GetX(), 5, checkpoints[i]->GetZ());
						cross->Scale(0.5);
						cross->RotateY(90);
						crosses.push_back(cross);
						currentSection = Complete;
					}
				}
			}

			//   CHECKPOINT COLLISION:   //
			for (int i = 0; i < checkpointDummies.size(); i++)
			{
				if (SphereToSphere(player, checkpointDummies[i], kShipRadius, 1))
				{
					currentHealth--;
					currentBounce = CalculateBounceDirection(player, checkpointDummies[i], momentum, 1, 1);
					CalculateBounce(player, currentBounce, momentum);
					currentBounce = None;
					currentHealth--;
				}
				else
				{
					currentBounce = None;
				}
			}

			//   TANK COLLISION:   //
			for (int i = 0; i < tankDummies.size(); i++)
			{
				if (SphereToSphere(player, tankDummies[i], kShipRadius, 8))
				{
					currentHealth--;
					currentBounce = CalculateBounceDirection(player, tankDummies[i], momentum, 3, 3);
					CalculateBounce(player, currentBounce, momentum);
					currentBounce = None;
					currentHealth--;
				}
				else
				{
					currentBounce = None;
				}
			}

			//   WALL COLLISION:   //
			for (int i = 0; i < walls.size(); i++)
			{
				if (SphereToBox(player, walls[i], kShipRadius, 2, 16))
				{
					currentBounce = CalculateBounceDirection(player, walls[i], momentum, 2, 16);
					CalculateBounce(player, currentBounce, momentum);
					currentBounce = None;
					currentHealth--;
				}
				else
				{
					currentBounce = None;
				}
			}
		}
		
		//   SYSTEM CONTROLS:   //
		if (engine->KeyHit(escape))
		{
			engine->Stop();
		}

		if (controller->ButtonHit(select))
		{
			engine->Stop();
		}

		if (engine->KeyHit(one))
		{
			currentCameraLocation = ThirdPerson;
			camera->SetPosition(player->GetX(), player->GetY(), player->GetZ());
			camera->Move(0, 10, -30);
			camera->ResetOrientation();
		}

		if (engine->KeyHit(two))
		{
			currentCameraLocation = FirstPerson;
			camera->SetPosition(player->GetX(), player->GetY(), player->GetZ());
			camera->Move(0, 5, 1);
			camera->ResetOrientation();
		}
	}
	engine->Delete();
}

////////////////////
//   FUNCTIONS:   //
////////////////////
void ReadTrackFile(string fileLocation)
{
	ifstream infile;
	infile.open(fileLocation);
	string line;

	while (!infile.eof())
	{
		const int defaultScale = 1;
		getline(infile, line);
		string read = "";
		string modelType = "";
		int spaceCount = 0;

		float x = 0;
		float y = 0;
		float z = 0;
		float rotationX = 0;
		float rotationY = 0;
		float rotationZ = 0;
		float scale = defaultScale;

		for (int i = 0; i < line.length(); i++)
		{
			if (line[i] == ' ')
			{
				if (spaceCount == 0)
				{
					modelType = read;
				}
				else if (spaceCount == 1)
				{
					x = stoi(read);
				}
				else if (spaceCount == 2)
				{
					y = stoi(read);
				}
				else if (spaceCount == 3)
				{
					z = stoi(read);
				}
				else if (spaceCount == 4)
				{
					rotationX = stoi(read);
				}
				else if (spaceCount == 5)
				{
					rotationY = stoi(read);
				}
				else if (spaceCount == 6)
				{
					rotationZ = stoi(read);
				}
				read = "";
				spaceCount++;
			}
			else if (i == line.length() - 1)
			{
				read += line[i];
				if (spaceCount == 7)
				{
					scale = stoi(read);
				}
				else if (spaceCount == 3)
				{
					z = stoi(read);
				}
				
				if (modelType == "CHECKPOINT")
				{
					IModel* model = meshes[3]->CreateModel(x, y, z);
					model->RotateX(rotationX);
					model->RotateY(rotationY);
					model->RotateZ(rotationZ);
					model->Scale(scale);
					checkpoints.push_back(model);

					IModel* dummy = meshes[9]->CreateModel(x, y, z);
					if (rotationY == 0)
					{
						dummy->MoveX(-17);
					}
					else
					{
						dummy->MoveZ(-17);
					}
					checkpointDummies.push_back(dummy);

					IModel* dummy2 = meshes[9]->CreateModel(x, y, z);
					if (rotationY == 0)
					{
						dummy2->MoveX(17);
					}
					else
					{
						dummy2->MoveZ(17);
					}
					checkpointDummies.push_back(dummy2);
				}
				else if (modelType == "ISLE")
				{
					IModel* model = meshes[4]->CreateModel(x, y, z);
					model->RotateX(rotationX);
					model->RotateY(rotationY);
					model->RotateZ(rotationZ);
					model->Scale(scale);
					isles.push_back(model);
				}
				else if (modelType == "WALL")
				{
					IModel* model = meshes[5]->CreateModel(x, y, z);
					model->RotateX(rotationX);
					model->RotateY(rotationY);
					model->RotateZ(rotationZ);
					model->Scale(scale);
					walls.push_back(model);
				}
				else if (modelType == "TANK")
				{
					int random = RandomGenerator(6, 7);
					IModel* model = meshes[random]->CreateModel(x, y, z);
					model->RotateX(rotationX);
					model->RotateY(rotationY);
					model->RotateZ(rotationZ);
					model->Scale(scale);
					tanks.push_back(model);

					IModel* dummy = meshes[9]->CreateModel(x, 0.0f, z);
					tankDummies.push_back(dummy);
				}
				else if (modelType == "WAYPOINT")
				{
					IModel* model = meshes[8]->CreateModel(x, y, z);
					waypoints.push_back(model);
				}
			}
			else
			{
				read += line[i];
			}
		}
	}
	infile.close();
}

bool PointToBox(IModel* point, IModel* box, int length, int width)
{
	float pointX = point->GetX();
	float pointZ = point->GetZ();
	float boxX = box->GetX();
	float boxZ = box->GetZ();

	if (pointX > (boxX - length) && pointX < (boxX + length))
	{
		if (pointZ > (boxZ - width) && pointZ < (boxZ + width))
		{
			return true;
		}
	}
	return false;
}

bool SphereToBox(IModel* sphere, IModel* box, int radius, int length, int width)
{
	float sphereX = sphere->GetX();
	float sphereZ = sphere->GetZ();

	float boxX = box->GetX();
	float boxZ = box->GetZ();

	if (sphereX > (boxX - length - radius) && sphereX < (boxX + length + radius))
	{
		if (sphereZ > (boxZ - width - radius) && sphereZ < (boxZ + width - radius))
		{
			return true;
		}
	}
	return false;
}

bool SphereToSphere(IModel* sphere, IModel* sphere2, int radius, int radius2)
{
	float sphereToSphereX = sphere->GetZ() - sphere2->GetZ();
	float sphereToSphereZ = sphere->GetX() - sphere2->GetX();

	float xSquare = sphereToSphereX * sphereToSphereX;
	float zSquare = sphereToSphereZ * sphereToSphereZ;
	float distance = sqrt(xSquare + zSquare);

	if (distance < (radius + radius2))
	{
		return true;
	}
	return false;
}

Direction CalculateBounceDirection(IModel* player, IModel* object, Vector2D momentum, float objectLength, float objectWidth)
{
	float playerX = player->GetX();
	float playerZ = player->GetZ();

	float objectMinX = object->GetX() - objectLength;
	float objectMaxX = object->GetX() + objectLength;
	float objectMinZ = object->GetZ() - objectWidth;
	float objectMaxZ = object->GetZ() + objectWidth;

	if (playerX > objectMinX && playerX < objectMaxX && playerZ < objectMinZ && playerZ < objectMaxZ)
	{	
		return Backward;
	}
	else if (playerX > objectMinX && playerX < objectMaxX && playerZ > objectMinZ && playerZ > objectMaxZ)
	{
		return Forward;
	}
	else if (playerX < objectMinX && playerX < objectMaxX && playerZ > objectMinZ && playerZ < objectMaxZ)
	{
		return Left;
	}
	else if (playerX > objectMinX && playerX > objectMaxX && playerZ > objectMinZ && playerZ < objectMaxZ)
	{
		return Right;
	}
	else
	{
		return Diagonal;
	}
}

void CalculateBounce(IModel* player, Direction direction, Vector2D momentum)
{
	if (direction == Forward)
	{
		player->Move(momentum.x, 0.0f, abs(momentum.z));
	}
	else if (direction == Backward)
	{
		player->Move(momentum.x, 0.0f, -abs(momentum.z));
	}
	else if (direction == Left)
	{
		player->Move(-abs(momentum.x), 0.0f, momentum.z);
	}
	else if (direction == Right)
	{
		player->Move(abs(momentum.x), 0.0f, momentum.z);
	}
	else if (direction == Diagonal)
	{
		player->Move(-abs(momentum.x), 0.0f, -abs(momentum.z));
	}
}

Vector2D Scalar(float scale, Vector2D vector)
{
	return { scale * vector.x, scale * vector.z };
}

Vector2D VectorAddition(Vector2D vector1, Vector2D vector2, Vector2D vector3)
{
	return { vector1.x + vector2.x + vector3.x, vector1.z + vector2.z + vector3.z };
}

int RandomGenerator(int minimum, int maximum)
{
	random_device generator;
	mt19937 twister(generator());
	uniform_int_distribution<> distribution(minimum, maximum);
	return distribution(twister);
}
