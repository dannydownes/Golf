#pragma once

#include "BasicActors.h"
#include <iostream>
#include <iomanip>
#include <stdlib.h>

namespace PhysicsEngine
{
	using namespace std;

	//a list of colours: Circus Palette
	static const PxVec3 color_palette[] =
	{ PxVec3(207.f / 255.f,	255.f / 255.f, 179.f / 255.f),	// green
		PxVec3(255.f / 255.f, 126.f / 255.f, 107.f / 255.f),	// red
		PxVec3(140.f / 255.f, 94.f / 255.f, 88.f / 255.f),	// brown
		PxVec3(255.f / 255.f, 200.f / 255.f, 87.f / 255.f),	// orange
		PxVec3(72.f / 255.f, 99.f / 255.f, 156.f / 255.f),	// blue
		PxVec3(255.f / 255.f, 255.f / 255.f, 255.f / 255.f), //white
		PxVec3(0.f / 255.f, 0.f / 255.f, 0.f / 255.f) //black
	};

	struct FilterGroup
	{
		enum Enum
		{
			Player = (1 << 0),
			Grass = (1 << 1),
			Ice = (1 << 2),
			Sides = (1 << 3)
		};
	};

	///A customised collision class, implemneting various callbacks
	class MySimulationEventCallback : public PxSimulationEventCallback
	{
	public:

		bool ballPotted;

		MySimulationEventCallback() : ballPotted(false) {};

		//an example variable that will be checked in the main simulation loop
		///Method called when the contact with the trigger object is detected.
		virtual void onTrigger(PxTriggerPair* pairs, PxU32 count)
		{
			//you can read the trigger information here
			for (PxU32 i = 0; i < count; i++)
			{
				//filter out contact with the planes
				if (pairs[i].otherShape->getGeometryType() != PxGeometryType::ePLANE)
				{
					if ((pairs[i].otherShape->getName() == "ball") &&
						(pairs[i].triggerShape->getName() == "hole"))
					{
						std::cout << "Ball Potted." << std::endl;
						ballPotted = true;
					}
				}
			}
		}

		///Method called when the contact by the filter shader is detected.
		virtual void onContact(const PxContactPairHeader &pairHeader, const PxContactPair *pairs, PxU32 nbPairs)
		{
			//check all pairs
			for (PxU32 i = 0; i < nbPairs; i++)
			{
				switch (pairs[i].shapes[1]->getSimulationFilterData().word0)
				{
				case FilterGroup::Ice:
					system("cls");
					std::cout << "Contact found between ball and ice." << std::endl;
					break;
				case FilterGroup::Grass:
					system("cls");
					std::cout << "Contact found between ball and grass" << std::endl;
					break;
				case FilterGroup::Sides:
					system("cls");
					std::cout << "Contact found between ball and sides" << std::endl;
					break;
				case FilterGroup::Player:
					system("cls");
					std::cout << "Contact found." << std::endl;
					break;
				default:
					std::cout << "No particle effects need to be played." << std::endl;
					break;
				}
			}
		}

		virtual void onConstraintBreak(PxConstraintInfo *constraints, PxU32 count) {}
		virtual void onWake(PxActor **actors, PxU32 count) {}
		virtual void onSleep(PxActor **actors, PxU32 count) {}
	};

	//A simple filter shader based on PxDefaultSimulationFilterShader - without group filtering
	static PxFilterFlags CustomFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		// let triggers through
		if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlags();
		}

		pairFlags = PxPairFlag::eCONTACT_DEFAULT;
		//enable continous collision detection
		pairFlags |= PxPairFlag::eSOLVE_CONTACT;
		pairFlags |= PxPairFlag::eDETECT_DISCRETE_CONTACT;
		pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;
		pairFlags |= PxPairFlag::eCCD_LINEAR;

		// trigger the contact callback for pairs (A,B) where 
		// the filtermask of A contains the ID of B and vice versa.
		if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		{
			//trigger onContact callback for this pair of objects
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;
			//pairFlags |= PxPairFlag::eNOTIFY_CONTACT_POINTS;
		}

		return PxFilterFlags();
	};

	///Custom scene class
	class MyScene : public Scene
	{
		MySimulationEventCallback* callback;

		Sphere* ball;
		PxRigidDynamic *Rball;
		bool onFloor = false;

		Plane* ground;

		PxMaterial *Grass = CreateMaterial(.6f, .6f, 0.1f);
		PxMaterial *Ice = CreateMaterial(0.0f, 0.0f, 0.0f);
		PxMaterial *Sides = CreateMaterial(.4f, .4f, .3f);
		PxMaterial *CourseGreen = CreateMaterial(.2f, .2f, 0.3f);

		//	Course
		SBox *bottom, *left, *right, *back;	//	Tee Area
		SBox *bottom1, *left1, *right1;		//	First Section	
		SBox *bottom2, *left2, *right2;		//	Second Section
		SBox *bottom3, *left3, *right3;		//	Jump Start
		SBox *bottom4, *left4, *right4;		//	Jump End
		SBox *bottom5, *left5, *right5;		//	Third Section
		SBox *bottom6, *left6, *right6;		//	Fourth Section
		SBox *bottom7, *leftfront7, *rightfront7, *left7, *right7, *leftback7, *rightback7;
		SBox *bottom8, *left8, *right8;		//	Fourth Section, After Windmill
		SBox *bottomleft9, *bottomright9, *bottomback9, *end, *hole, *pole; // End Section

		//	End Zone
		Cloth *flag;	
		PxCloth *pFlag;
		PxReal windX, windZ;

		//SLIDER
		PxRigidDynamic *Rlift;
		DBox *lift;		//	Fifth Section
		PxPrismaticJoint *jJump;
		int slideSpeed = 1;
		int holder = slideSpeed;
		bool moveUp = true;
		bool moveDown = false;

		//	Windmill
		PxRevoluteJoint *jBlades;
		Windmill *wind;
		PyramidStatic *pyramid;
		Cross *blades;
		PxRigidDynamic *Rblades;

	public:
		PxTransform validBallPosition;
		MyScene() : Scene(CustomFilterShader) {};

		///A custom scene class
		void SetVisualisation()
		{
			px_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);

			px_scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);

			//cloth visualisation
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_HORIZONTAL, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_VERTICAL, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_BENDING, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_SHEARING, 1.0f);
		}

		//Custom scene initialisation
		virtual void CustomInit()
		{
			SetVisualisation();
			callback = new MySimulationEventCallback();
			px_scene->setSimulationEventCallback(callback);
			px_scene->setFlag(PxSceneFlag::eENABLE_CCD, true);

			//for (size_t i = 0; i < 2; i++)
			//{
				// BALL
			ball = new Sphere(PxTransform(PxVec3(0.0f, 1.0f, 0.0f)));
			ball->Color(color_palette[5]);
			ball->GetShape(0)->setGeometry(PxSphereGeometry(.1f));
			Rball = (PxRigidDynamic*)ball->Get();
			Rball->setMass(2.f);
			Rball->setMassSpaceInertiaTensor(PxVec3(.0f));
			ball->SetupFiltering(FilterGroup::Player, FilterGroup::Ice | FilterGroup::Grass | FilterGroup::Sides);
			Rball->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
			ball->GetShape(0)->setName("ball");
			Add(ball);

			// GROUND
			ground = new Plane();
			ground->Material(Grass);
			ground->Color(color_palette[0]);
			Add(ground);

			// TEE AREA
			bottom = new SBox(PxTransform(PxVec3(.0f, .3f, .0f)));
			bottom->Material(CourseGreen);
			bottom->Color(color_palette[3]);
			bottom->GetShape(0)->setGeometry(PxBoxGeometry(1.0f, 0.3f, 1.0f));
			bottom->SetupFiltering(FilterGroup::Grass, FilterGroup::Player);
			Add(bottom);

			left = new SBox(PxTransform(PxVec3(-1.2f, .5f, .0f)));
			left->Color(color_palette[1]);
			left->Material(Sides);
			left->GetShape(0)->setGeometry(PxBoxGeometry(.2f, 0.5f, 1.0f));
			left->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(left);


			right = new SBox(PxTransform(PxVec3(1.2f, 0.5f, .0f)));
			right->Color(color_palette[1]);
			right->Material(Sides);
			right->GetShape(0)->setGeometry(PxBoxGeometry(.2f, 0.5f, 1.0f));
			right->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(right);

			back = new SBox(PxTransform(PxVec3(0.f, 0.5f, 1.2f)));
			back->Color(color_palette[1]);
			back->Material(Sides);
			back->GetShape(0)->setGeometry(PxBoxGeometry(1.4f, 0.5f, .2f));
			back->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(back);

			// FIRST
			bottom1 = new SBox(PxTransform(PxVec3(.0f, .3f, -5.0f)));
			bottom1->Color(color_palette[0]);
			bottom1->Material(CourseGreen);
			bottom1->GetShape(0)->setGeometry(PxBoxGeometry(1.0f, 0.3f, 4.0f));
			bottom1->SetupFiltering(FilterGroup::Grass, FilterGroup::Player);
			Add(bottom1);

			left1 = new SBox(PxTransform(PxVec3(-1.2f, 0.5f, -5.0f)));
			left1->Color(color_palette[1]);
			left1->Material(Sides);
			left1->GetShape(0)->setGeometry(PxBoxGeometry(.2f, 0.5f, 4.0f));
			left1->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(left1);

			right1 = new SBox(PxTransform(PxVec3(1.2f, 0.5f, -6.2f)));
			right1->Color(color_palette[1]);
			right1->Material(Sides);
			right1->GetShape(0)->setGeometry(PxBoxGeometry(.2f, 0.5f, 5.2f));
			right1->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(right1);

			// SECOND
			bottom2 = new SBox(PxTransform(PxVec3(-3.f, .297f, -10.f)));
			bottom2->Material(Ice);
			bottom2->Color(color_palette[4]);
			bottom2->GetShape(0)->setGeometry(PxBoxGeometry(4.0f, 0.3f, 1.0f));
			bottom2->SetupFiltering(FilterGroup::Ice, FilterGroup::Player);
			Add(bottom2);

			left2 = new SBox(PxTransform(PxVec3(-4.2f, 0.5f, -8.8f)));
			left2->Color(color_palette[1]);
			left2->Material(Sides);
			left2->GetShape(0)->setGeometry(PxBoxGeometry(2.8f, 0.5f, .2f));
			left2->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(left2);

			right2 = new SBox(PxTransform(PxVec3(-3.0f, 0.5f, -11.2f)));
			right2->Color(color_palette[1]);
			right2->Material(Sides);
			right2->GetShape(0)->setGeometry(PxBoxGeometry(4.f, 0.5f, .2f));
			right2->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(right2);

			// Jump Start
			bottom3 = new SBox(PxTransform(PxVec3(-7.955f, 1.015f, -10.f), PxQuat(-0.7f, PxVec3(.0f, .0f, 1.f))));
			bottom3->Color(color_palette[4]);
			bottom3->Material(Ice);
			bottom3->GetShape(0)->setGeometry(PxBoxGeometry(1.0f, 0.3f, 1.0f));
			bottom3->SetupFiltering(FilterGroup::Ice, FilterGroup::Player);
			Add(bottom3);

			left3 = new SBox(PxTransform(PxVec3(-7.89f, 1.315f, -8.8f), PxQuat(-0.7f, PxVec3(.0f, .0f, 1.f))));
			left3->Color(color_palette[1]);
			left3->Material(Sides);
			left3->GetShape(0)->setGeometry(PxBoxGeometry(1.0f, 0.5f, .2f));
			left3->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(left3);

			right3 = new SBox(PxTransform(PxVec3(-7.89f, 1.315f, -11.2f), PxQuat(-0.7f, PxVec3(.0f, .0f, 1.f))));
			right3->Color(color_palette[1]);
			right3->Material(Sides);
			right3->GetShape(0)->setGeometry(PxBoxGeometry(1.0f, 0.5f, .2f));
			right3->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(right3);

			lift = new DBox(PxTransform(PxVec3(-9.8f, 1.f, -10.f), PxQuat(1.5708f, PxVec3(1.0f, 0.0f, .0f))));
			lift->Color(color_palette[2]);
			lift->Material(Sides);
			lift->GetShape(0)->setGeometry(PxBoxGeometry(0.2f, 2.f, 1.4f));
			Rlift = (PxRigidDynamic*)lift->Get();
			Rlift->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, true);
			//Rlift->setMass(.0f); //KEEP COMMENTED, SHITS ALL OVER CCD
			lift->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(lift);
			jJump = PxPrismaticJointCreate(*GetPhysics(), NULL, PxTransform(PxVec3(-9.8f, 5.0f, -10.0f)), lift->GetShape(0)->getActor(), PxTransform(PxVec3(0.f, .0f, .0f)));
			jJump->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);

			// Jump End
			bottom4 = new SBox(PxTransform(PxVec3(-11.955f, 1.015f, -10.f), PxQuat(0.7f, PxVec3(.0f, .0f, 1.f))));
			bottom4->Color(color_palette[0]);
			bottom4->Material(CourseGreen);
			bottom4->GetShape(0)->setGeometry(PxBoxGeometry(1.0f, 0.3f, 1.0f));
			bottom4->SetupFiltering(FilterGroup::Grass, FilterGroup::Player);
			Add(bottom4);

			left4 = new SBox(PxTransform(PxVec3(-12.03f, 1.315f, -8.8f), PxQuat(0.7f, PxVec3(.0f, .0f, 1.f))));
			left4->Color(color_palette[1]);
			left4->Material(Sides);
			left4->GetShape(0)->setGeometry(PxBoxGeometry(1.0f, 0.5f, .2f));
			left4->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(left4);

			right4 = new SBox(PxTransform(PxVec3(-12.03f, 1.315f, -11.2f), PxQuat(0.7f, PxVec3(.0f, .0f, 1.f))));
			right4->Color(color_palette[1]);
			right4->Material(Sides);
			right4->GetShape(0)->setGeometry(PxBoxGeometry(1.0f, 0.5f, .2f));
			right4->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(right4);

			// Third
			bottom5 = new SBox(PxTransform(PxVec3(-16.7f, .3f, -10.f)));
			bottom5->Color(color_palette[0]);
			bottom5->Material(CourseGreen);
			bottom5->GetShape(0)->setGeometry(PxBoxGeometry(3.8f, 0.3f, 1.0f));
			bottom5->SetupFiltering(FilterGroup::Grass, FilterGroup::Player);
			Add(bottom5);

			left5 = new SBox(PxTransform(PxVec3(-16.9f, 0.5f, -8.8f)));
			left5->Color(color_palette[1]);
			left5->Material(Sides);
			left5->GetShape(0)->setGeometry(PxBoxGeometry(4.f, 0.5f, .2f));
			left5->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(left5);

			right5 = new SBox(PxTransform(PxVec3(-15.7f, 0.5f, -11.2f)));
			right5->Material(Sides);
			right5->Color(color_palette[1]);
			right5->GetShape(0)->setGeometry(PxBoxGeometry(2.8f, 0.5f, .2f));
			right5->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(right5);

			// Fourth
			bottom6 = new SBox(PxTransform(PxVec3(-19.5f, .3f, -13.f)));
			bottom6->Color(color_palette[0]);
			bottom6->Material(CourseGreen);
			bottom6->GetShape(0)->setGeometry(PxBoxGeometry(1.f, 0.3f, 2.0f));
			bottom6->SetupFiltering(FilterGroup::Grass, FilterGroup::Player);
			Add(bottom6);

			left6 = new SBox(PxTransform(PxVec3(-20.7f, 0.5f, -12.f)));
			left6->Color(color_palette[1]);
			left6->Material(Sides);
			left6->GetShape(0)->setGeometry(PxBoxGeometry(.2f, 0.5f, 3.f));
			left6->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(left6);

			right6 = new SBox(PxTransform(PxVec3(-18.3f, 0.5f, -13.2f)));
			right6->Color(color_palette[1]);
			right6->Material(Sides);
			right6->GetShape(0)->setGeometry(PxBoxGeometry(.2f, 0.5f, 1.8f));
			right6->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(right6);

			// Fourth
			bottom7 = new SBox(PxTransform(PxVec3(-19.5f, .3f, -23.f)));
			bottom7->Color(color_palette[0]);
			bottom7->Material(CourseGreen);
			bottom7->GetShape(0)->setGeometry(PxBoxGeometry(8.f, 0.3f, 8.0f));
			bottom7->SetupFiltering(FilterGroup::Grass, FilterGroup::Player);
			Add(bottom7);

			leftfront7 = new SBox(PxTransform(PxVec3(-24.2f, 0.5f, -14.8f)));
			leftfront7->Color(color_palette[1]);
			leftfront7->Material(Sides);
			leftfront7->GetShape(0)->setGeometry(PxBoxGeometry(3.3f, 0.5f, .2f));
			leftfront7->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(leftfront7);

			rightfront7 = new SBox(PxTransform(PxVec3(-14.8f, 0.5f, -14.8f)));
			rightfront7->Color(color_palette[1]);
			rightfront7->Material(Sides);
			rightfront7->GetShape(0)->setGeometry(PxBoxGeometry(3.3f, 0.5f, .2f));
			rightfront7->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(rightfront7);

			left7 = new SBox(PxTransform(PxVec3(-27.7f, 0.5f, -23.f)));
			left7->Color(color_palette[1]);
			left7->Material(Sides);
			left7->GetShape(0)->setGeometry(PxBoxGeometry(.2f, 0.5f, 8.4f));
			left7->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(left7);

			right7 = new SBox(PxTransform(PxVec3(-11.3f, 0.5f, -23.f)));
			right7->Color(color_palette[1]);
			right7->Material(Sides);
			right7->GetShape(0)->setGeometry(PxBoxGeometry(.2f, 0.5f, 8.4f));
			right7->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(right7);

			leftback7 = new SBox(PxTransform(PxVec3(-24.2f, 0.5f, -31.2f)));
			leftback7->Color(color_palette[1]);
			leftback7->Material(Sides);
			leftback7->GetShape(0)->setGeometry(PxBoxGeometry(3.3f, 0.5f, .2f));
			leftback7->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(leftback7);

			rightback7 = new SBox(PxTransform(PxVec3(-14.8f, 0.5f, -31.2f)));
			rightback7->Color(color_palette[1]);
			rightback7->Material(Sides);
			rightback7->GetShape(0)->setGeometry(PxBoxGeometry(3.3f, 0.5f, .2f));
			rightback7->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(rightback7);

			//Fifth
			bottom8 = new SBox(PxTransform(PxVec3(-19.5f, .3f, -35.f)));
			bottom8->Color(color_palette[0]);
			bottom8->Material(CourseGreen);
			bottom8->GetShape(0)->setGeometry(PxBoxGeometry(1.f, 0.3f, 4.0f));
			bottom8->SetupFiltering(FilterGroup::Grass, FilterGroup::Player);
			Add(bottom8);

			left8 = new SBox(PxTransform(PxVec3(-20.7f, 0.5f, -35.65f)));
			left8->Color(color_palette[1]);
			left8->Material(Sides);
			left8->GetShape(0)->setGeometry(PxBoxGeometry(.2f, 0.5f, 4.65f));
			left8->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(left8);

			right8 = new SBox(PxTransform(PxVec3(-18.3f, 0.5f, -35.65f)));
			right8->Color(color_palette[1]);
			right8->Material(Sides);
			right8->GetShape(0)->setGeometry(PxBoxGeometry(.2f, 0.5f, 4.65f));
			right8->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(right8);

			// WINDMILL
			wind = new Windmill(PxTransform(PxVec3(-19.5f, 0.f, -31.8f)));
			wind->GetShape(0)->setLocalPose(PxTransform(PxVec3(.8f, .9f, .0f)));
			wind->GetShape(1)->setLocalPose(PxTransform(PxVec3(.0f, .9f, .0f)));
			wind->GetShape(2)->setLocalPose(PxTransform(PxVec3(-0.8f, .9f, .0f)));
			wind->GetShape(3)->setLocalPose(PxTransform(PxVec3(0.f, 2.2f, .0f)));
			wind->Material(Sides);
			wind->Color(color_palette[3]);
			wind->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(wind);

			// ROOF
			pyramid = new PyramidStatic(PxTransform(PxVec3(-19.5f, 3.2f, -31.8f), PxQuat(1.5708f / 2.0f, PxVec3(.0f, 1.0f, .0f))));
			pyramid->Color(color_palette[2]);
			Add(pyramid);

			// WINDMILL BLADES
			blades = new Cross(PxTransform(PxVec3(-19.5f, 2.65f, -30.8f)));
			blades->Material(Sides);
			blades->GetShape(0)->setGeometry(PxBoxGeometry(.3f, 2.f, .1f));
			blades->GetShape(1)->setGeometry(PxBoxGeometry(2.f, .3f, .1f));
			blades->Color(color_palette[2]);
			Rblades = (PxRigidDynamic*)blades->Get();
			//Rblades->setMass(0.f); //KEEP COMMENTED, SHITS ALL OVER CCD
			blades->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(blades);
			jBlades = PxRevoluteJointCreate(*GetPhysics(), NULL, PxTransform(PxVec3(-19.5f, 2.65f, -30.8f), PxQuat(1.5708f * 3, PxVec3(.0f, 1.0f, .0f))), (PxRigidActor*)blades->Get(), PxTransform(PxVec3(0.f, .0f, .0f), PxQuat(1.5708f * 1, PxVec3(.0f, 1.0f, .0f))));
			jBlades->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
			if (Rblades->isSleeping())
			{
				Rblades->wakeUp();
			}
			jBlades->setDriveVelocity(1.5f);
			jBlades->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);

			bottomleft9 = new SBox(PxTransform(PxVec3(-20.15f, .3f, -39.35f)));
			bottomleft9->Color(color_palette[0]);
			bottomleft9->Material(CourseGreen);
			bottomleft9->GetShape(0)->setGeometry(PxBoxGeometry(.35f, 0.3f, .35f));
			bottomleft9->SetupFiltering(FilterGroup::Grass, FilterGroup::Player);
			Add(bottomleft9);

			bottomright9 = new SBox(PxTransform(PxVec3(-18.85f, .3f, -39.35f)));
			bottomright9->Color(color_palette[0]);
			bottomright9->Material(CourseGreen);
			bottomright9->GetShape(0)->setGeometry(PxBoxGeometry(.35f, 0.3f, .35f));
			bottomright9->SetupFiltering(FilterGroup::Grass, FilterGroup::Player);
			Add(bottomright9);

			bottomback9 = new SBox(PxTransform(PxVec3(-19.5f, .3f, -40.f)));
			bottomback9->Color(color_palette[0]);
			bottomback9->Material(CourseGreen);
			bottomback9->GetShape(0)->setGeometry(PxBoxGeometry(1.f, 0.3f, .3f));
			bottomback9->SetupFiltering(FilterGroup::Grass, FilterGroup::Player);
			Add(bottomback9);

			end = new SBox(PxTransform(PxVec3(-19.5f, 0.5f, -40.5f)));
			end->Color(color_palette[1]);
			end->Material(Sides);
			end->GetShape(0)->setGeometry(PxBoxGeometry(1.4f, 0.5f, .2f));
			left4->SetupFiltering(FilterGroup::Sides, FilterGroup::Player);
			Add(end);

			hole = new SBox(PxTransform(PxVec3(-19.5f, .2f, -39.35f)));
			hole->Color(color_palette[6]);
			hole->GetShape(0)->setGeometry(PxBoxGeometry(.3f, 0.2f, .35f));
			hole->GetShape(0)->setName("hole");
			Add(hole);
			hole->SetTrigger(true, 0);

			pole = new SBox(PxTransform(PxVec3(-19.5f, 1.25f, -39.35f)));
			pole->Color(color_palette[6]);
			pole->GetShape(0)->setGeometry(PxBoxGeometry(.02f, 1.25f, .02f));
			pole->GetShape(0)->getActor()->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, true);
			Add(pole);

			flag = new Cloth(PxTransform(PxVec3(-19.48f, 2.f, -39.35f), PxQuat(1.5708f, PxVec3(0.0f, 0.0f, 1.0f))), PxVec2(.5f, .7f), 50, 50);
			flag->Color(color_palette[3]);
			Add(flag);
			pFlag = (PxCloth*)flag->Get();
			pFlag->setExternalAcceleration(PxVec3(10.0f, 0.5f, 0.0f));
			pFlag->setSelfCollisionDistance(0.01f);
			pFlag->setStretchConfig(PxClothFabricPhaseType::eSHEARING, PxClothStretchConfig(0.5f));
			pFlag->setDampingCoefficient(PxVec3(.1f, .1f, .1f));
			pFlag->setDragCoefficient(0.05f);

			//for (size_t i = 0; i < 99; i++)
			//{
			//	Cloth* flag = new Cloth(PxTransform(PxVec3(0, 5.f, i), PxQuat(1.5708f, PxVec3(0.0f, 0.0f, 1.0f))), PxVec2(.5f, .7f), 50, 50);
			//	flag->Color(color_palette[3]);
			//	Add(flag);
			//	PxCloth *pFlag = (PxCloth*)flag->Get();
			//	pFlag->setExternalAcceleration(PxVec3(10.0f, 0.5f, 0.0f));
			//	pFlag->setSelfCollisionDistance(0.01f);
			//	pFlag->setStretchConfig(PxClothFabricPhaseType::eSHEARING, PxClothStretchConfig(0.5f));
			//	pFlag->setDampingCoefficient(PxVec3(.1f, .1f, .1f));
			//	pFlag->setDragCoefficient(0.05f);
			//}
		//}
		}

		//Custom udpate function
		virtual void CustomUpdate()
		{
			windX = static_cast<float>(rand() % 20);
			windZ = static_cast<float>(rand() % 20);
			pFlag->setExternalAcceleration(PxVec3(windX, 1.0f, windZ));

			if (Rball->getGlobalPose().p.y < 0.3f && callback->ballPotted == false)
			{
				Rball->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));
				Rball->setGlobalPose(validBallPosition);
			}
			else if (Rball->getGlobalPose().p.y > 0.3f && (Rball->getLinearVelocity() == PxVec3(0, 0, 0)))
			{
				validBallPosition = Rball->getGlobalPose();
			}

			PxVec3 pos;
			pos = Rlift->getGlobalPose().p;
			if (pos.y > 1.2f && moveDown == false)
			{
				moveUp = false;
				holder = slideSpeed * (-1);
				moveDown = true;
			}

			if (pos.y < -0.5f && moveUp == false)
			{
				moveDown = false;
				holder = slideSpeed;
				moveUp = true;

			}
			Rlift->setGlobalPose(PxTransform(PxVec3(pos.x, (pos.y + (0.01f * holder)), pos.z), PxQuat(1.5708f, PxVec3(1.0f, 0.0f, .0f))));
		}

		bool GetPotted()
		{
			return callback->ballPotted;
		}
	};
}
