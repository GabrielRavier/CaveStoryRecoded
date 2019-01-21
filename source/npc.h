#pragma once

#include <deque>
#include <array>
#include "common.h"
#include "log.h"

struct NPC_RECT
{
	uint8_t front;
	uint8_t top;
	uint8_t back;
	uint8_t bottom;
};

struct NPC_TABLE
{
	uint16_t bits;
	uint16_t life;
	uint8_t surf;
	uint8_t hit_voice;
	uint8_t destroy_voice;
	uint8_t size;
	int32_t exp;
	int32_t damage;
	NPC_RECT hit;
	NPC_RECT view;
};

extern NPC_TABLE *npcTable;

class npc
{
	//Variables
public:
	uint8_t cond;	// Current condition of the NPC (alive or dead)
	int flag;	// Flag for collision

	int x;	// X position
	int y;	// Y position

	int xm;	// Usually used as the X velocity
	int ym;	// Usually used as the Y velocity
	int xm2;	// 2nd X velocity ?
	int ym2;	// 2nd Y velocity ?

	int tgt_x;	// Target X (also often used to save some random value)
	int tgt_y;	// Target Y (also often used to save some random value)

	int code_char;	// ID of the NPC
	int code_flag;	// Flag associated with the NPC
	int code_event;	// Event associated with the NPC's death

	int surf;	// Spritesheet

	int hit_voice;	// Sound emitted when getting hit
	int destroy_voice;	// Sound emitted when dying

	int life;	// HP of the NPC
	int exp;	// Number of EXP dropped when the NPC dies

	int size;	// Size of the explosion made when dying

	int direct;	// Direction the NPC is facing towards

	uint16_t bits;	// Entity flags

	RECT rect;	// Location of the NPC's texture in the spritesheet

	int ani_wait;	// Usually used as a wait timer for animation
	int ani_no;	// Usually used as the current animation ID
	int count1;	// Usually used as a counter
	int count2;	// Usually used as a counter
	int act_no;	// Usually used as the current state
	int act_wait;	// Usually used as a timer for the state

	RECT hit;	// RECT delimiting the hitbox of the NPC

	RECT view;	// Offset ?

	uint8_t shock;	// Current time left invincible
	int damage_view;	// ???
	int damage;	// Amount of damage dealt when touching the player

	npc *pNpc;	// Parent NPC

	bool priority;	// Priority

public:
	// These are kinda supposed to be internal, but I can't put them as protected/private
	void accelerateTowardsPlayer(int vel);
	void animate(int aniWait, int aniStart = -1, int aniMax = -1);
	void animateReverse(int aniWait, int aniStart = -1, int aniMin = -1);
	void createSmokeWithVel(size_t num, int xVel, int yVel);
	void doGravity(int gravity, int maxYVel);

	template <size_t N> void doRects(const std::array<RECT, N>& rcLeft, const std::array<RECT, N>& rcRight)
	{
		try
		{
			if (this->direct != dirLeft)
				this->rect = rcRight.at(this->ani_no);
			else
				this->rect = rcLeft.at(this->ani_no);
		}
		catch (const std::out_of_range& oops)
		{
			logError("Out of range exception when trying to handle rects (NPC number " + std::to_string(this->code_char) +
				" and attempted access to rect number " + std::to_string(this->ani_no) + ") (exception details : " + oops.what() + ')');
			this->surf = 0x27;
			this->doRects({ 0, 0, this->view.left >> 8, this->view.top >> 8 });
		}
	}
	template <size_t N> void doRects(const std::array<RECT, N>& rcNPC)
	{
		try
		{
			this->rect = rcNPC.at(this->ani_no);
		} catch (const std::out_of_range& lmaoUFuckedUp)
		{
			logError("Out of range exception when trying to handle rects (NPC number "  + std::to_string(this->code_char) +
				" and attempted access to rect number " + std::to_string(this->ani_no) + ") (exception details : " + lmaoUFuckedUp.what() + ')');
			this->surf = 0x27;
			this->doRects({ 0, 0, this->view.left >> 8, this->view.top >> 8 });
		}
	}

	void doRects(const RECT& rcLeft, const RECT& rcRight);
	void doRects(const RECT& rcNPC);

	void facePlayer();
	attrPure int getXDistToPlayer();

	void limitXVel(int maxVel);
	void limitYVel(int maxVel);

	void limitXVel2(int maxVel);
	void limitYVel2(int maxVel);

	void moveInDir(int vel);

	void accelerateTowardsXTarget(int vel);
	void accelerateTowardsYTarget(int vel);

	attrPure bool isPlayerWithinDistance(int xDist, int yDistHigh, int yDistLow);
	attrPure inline bool isPlayerWithinDistance(int xDist, int yDist)
	{
		return isPlayerWithinDistance(xDist, yDist, yDist);
	}

	attrPure bool isPlayerAligned(int xRay, int yRayHigh, int yRayLow);
	attrPure inline bool isPlayerAligned(int xRay, int yRay)
	{
		return isPlayerAligned(xRay, yRay, yRay);
	}

	void init(int setCode, int setX, int setY, int setXm, int setYm, int setDir, npc *parentNpc);

	void update();
	void draw();
};

void loadNpcTable();

void createSmokeLeft(int x, int y, int w, size_t num);
void createSmokeUp(int x, int y, int w, int num);

void createNpc(int setCode, int setX = 0, int setY = 0, int setXm = 0, int setYm = 0, int setDir = dirLeft, npc *parentNpc = nullptr, bool setPriority = true);
void changeNpc(int code_event, int code_char, int dir = dirLeft);
attrPure int findEntityByType(int entityType);
void setNPCState(int entityEventNum, int newNPCState, int direction);
void moveNPC(int entityEventNum, int xPos, int yPos, int direction);

void updateNPC();
void drawNPC();
void dropExperience(int x, int y, int exp);
void killNpc(npc *NPC, bool bVanish = true);
void killNpcsByType(int entityType, bool makeDustClouds = true);

extern std::deque<npc> npcs;

extern int superXPos;
extern int superYPos;

extern int curlyShootWait;
extern int curlyShootX;
extern int curlyShootY;

enum NPC_cond
{
	npccond_dmgboss = 0x10, // When set damages the main boss
	npccond_alive = 0x80, // Determines if an npc is alive
};

enum NPC_flags
{
	npc_solidSoft = 0x1, // Pushes quote out
	npc_ignore44 = 0x2, // Ignores tile 44 (No NPC)
	npc_invulnerable = 0x4, // Can't get hit
	npc_ignoreSolid = 0x8, // Doesn't collide with anything
	npc_bouncy = 0x10, // Quote bounces on the top
	npc_shootable = 0x20, // Can be shot
	npc_solidHard = 0x40, // Essentially acts as level tiles
	npc_rearTop = 0x80, // Rear and top don't hurt
	npc_eventTouch = 0x100, // Run event when touched
	npc_eventDie = 0x200, // Run event when killed
	npc_appearSet = 0x800, // Only appear when flag is set
	npc_altDir = 0x1000, // Spawn facing to the right (or however the npc interprets the direction as)
	npc_interact = 0x2000, // Run event when interacted with
	npc_hideSet = 0x4000, // Hide when flag is set
	npc_showDamage = 0x8000, // Show #Damage taken
};

enum NPCNames
{
	NPC_Null = 0,	// Direction 2 = Moves 1 block down immediately when spawned
	NPC_EXP = 1,	// Weapon energy
	NPC_EnemyBehemoth = 2,
	NPC_NullDeletesItself = 3,	// Lasts 2 seconds, used to hold and indicate the position damage values displayed after an NPC is killed
	NPC_Smoke = 4,
	NPC_EnemyCritterHoppingGreen = 5,
	NPC_EnemyBeetleHorizontalGreen = 6,
	NPC_EnemyBasil = 7,
	NPC_EnemyBeetleFollowGreen = 8,
	NPC_BalrogFalling = 9,
	NPC_BossBalrogShooting = 10,
	NPC_ProjectileBalrogEnergyBallInvincible = 11,
	NPC_BalrogStanding = 12,
	NPC_Forcefield = 13,
	NPC_SantasKey = 14,
	NPC_TreasureChestClosed = 15,	// Affected by gravity
	NPC_SavePoint = 16,
	NPC_HealthAmmoRefill = 17,
	NPC_Door = 18,
	NPC_BalrogBustsIn = 19,
	NPC_Computer = 20,	// Direction 0 = Screen off, 2 = Screen on
	NPC_TreasureChestOpen = 21,	// Not affected by gravity
	NPC_Teleporter = 22,
	NPC_TeleporterLights = 23,
	NPC_EnemyPowerCritter = 24,
	NPC_LiftPlatform = 25,
	NPC_EnemyBatBlackCircling = 26,
	NPC_DeathSpikes = 27,
	NPC_EnemyCritterFlyingGreen = 28,
	NPC_Cthulhu = 29,
	NPC_HermitGunsmith = 30,
	NPC_EnemyBatBlackHanging = 31,
	NPC_LifeCapsule = 32,
	NPC_ProjectileBalrogEnergyBallBouncing = 33,
	NPC_Bed = 34,
	NPC_EnemyManann = 35,
	NPC_BossBalrogFlying = 36,
	NPC_Signpost = 37,
	NPC_Fireplace = 38,
	NPC_CocktailSign = 39,
	NPC_Santa = 40,
	NPC_BustedDoorway = 41,
	NPC_Sue = 42,
	NPC_BlackboardTable = 43,	// Direction 0 = Blackboard, 2 = Table and poster
	NPC_EnemyPolish = 44,
	NPC_EnemyBaby = 45,
	NPC_HorizontalVerticalTrigger = 46,	// Direction 0 = Horizontal, 2 = Vertical. Moves at 0.1873779296875 blocks per frame, or 9.368896484375 blocks per second
	NPC_EnemySandcrocGreen = 47,
	NPC_ProjectileOmegaMudball = 48,
	NPC_EnemySkullhead = 49,
	NPC_ProjectileBone = 50,
	NPC_EnemyCrowAndSkullhead = 51,
	NPC_BlueRobotStanding = 52,
	NPC_EnemySkullstepLeg = 53,	// Leg/Foot
	NPC_EnemySkullstep = 54,
	NPC_Kazuma = 55,
	NPC_EnemyBeetleHorizontalBrown = 56,
	NPC_EnemyCrow = 57,
	NPC_EnemyBasu1 = 58,
	NPC_EnemyEvilDoor = 59,
	NPC_Toroko = 60,
	NPC_King = 61,
	NPC_KazumaFacingAway = 62,	// Usually when he's using a computer
	NPC_TorokoPanicking = 63,	// When she attacks you in the shed
	NPC_EnemyCritterHoppingBlue = 64,
	NPC_EnemyBatBlueVertical = 65,
	NPC_ProjectileMiseryBubble = 66,
	NPC_MiseryFlying = 67,	// When she's floating/flying in cinematics
	NPC_BossBalrogRunning = 68,
	NPC_EnemyPignon = 69,
	NPC_SparklingItem = 70,
	NPC_EnemyChinfish = 71,
	NPC_Sprinkler = 72,	// Direction 0 = On, 2 = Off
	NPC_WaterDrop1 = 73,
	NPC_Jack = 74,
	NPC_KanpachiFishing = 75,
	NPC_Flowers = 76,
	NPC_SandaimesPavillon = 77,
	NPC_Pot = 78,
	NPC_Mahin = 79,
	NPC_EnemyGravekeeper = 80,
	NPC_EnemyGiantPignon = 81,
	NPC_MiseryStanding = 82,
	NPC_IgorStanding = 83,	// For cutscene
	NPC_ProjectileBasuEnergyBall1 = 84,
	NPC_Terminal = 85,	// Direction 2 = Red Screen
	NPC_Missile = 86,	// Direction 2 = Doesn't despawn, EXP = health get
	NPC_Heart = 87,	// Direction 2 = Doesn't despawn, EXP = ammo get
	NPC_BossIgor = 88,
	NPC_IgorDying = 89,	// When defeated
	NPC_Background = 90,
	NPC_Cage = 91,
	NPC_SueFacingAway = 92,	// USually when she's using a computer
	NPC_Chaco = 93,
	NPC_EnemyKulala = 94,
	NPC_EnemyJelly = 95,
	NPC_FanLeft = 96,	// Direction 2 = On
	NPC_FanUp = 97,	// Direction 2 = On
	NPC_FanRight = 98,	// Direction 2 = On
	NPC_FanDown = 99,	// Direction 2 = On
	NPC_Grate = 100,
	NPC_PowerControlsScreen = 101,
	NPC_PowerControlsPowerFlow = 102,
	NPC_ProjectileMannan = 0x67,
	NPC_EnemyFrog = 0x68,
	NPC_BalloonHeyLow = 0x69,
	NPC_BalloonHeyHigh = 0x6A,
	NPC_MalcoUndamaged = 0x6B,
	NPC_ProjectileBalfrog = 0x6C,
	NPC_MalcoDamaged = 0x6D,
	NPC_EnemyPuchi = 0x6E,
	NPC_QuoteTeleportsOut = 0x6F,
	NPC_QuoteTeleportsIn = 0x70,
	NPC_ProfessorBooster = 0x71,
	NPC_EnemyPress = 0x72,
	NPC_EnemyRavil = 0x73,
	NPC_RedFlowersPetals = 0x74,
	NPC_Curly = 0x75,
	NPC_BossCurly = 0x76,
	NPC_TableChair = 0x77,
	NPC_Colon1 = 0x78,
	NPC_Colon2 = 0x79,
	NPC_EnemyColon = 0x7A,
	NPC_ProjectileCurly = 0x7B,
	NPC_Sunstone = 0x7C,
	NPC_HiddenHeartMissile = 0x7D,
	NPC_PuppyRunsAway = 0x7E,
	NPC_ProjectileMachineGunTrailLevel2 = 0x7F,
	NPC_ProjectileMachineGunTrailLevel3 = 0x80,
	NPC_ProjectileFireballSnakeTrail = 0x81,
	NPC_PuppyTailWag = 0x82,
	NPC_PuppySleeping = 0x83,
	NPC_PuppyBark = 0x84,
	NPC_Jenka = 0x85,
	NPC_EnemyArmadillo = 0x86,
	NPC_EnemySkeleton = 0x87,
	NPC_PuppyCarried = 0x88,
	NPC_LargeDoorwayFrame = 0x89,
	NPC_LargeDoorwayDoors = 0x8A,
	NPC_DoctorWithCrown = 0x8B,
	NPC_BossFrenziedToroko = 0x8C,
	NPC_ProjectileTorokoBlock = 0x8D,
	NPC_EnemyFlowercub = 0x8E,
	NPC_JenkaCollapsed = 0x8F,
	NPC_TorokoTeleportsIn = 0x90,
	NPC_KingStruckByLightning = 0x91,
	NPC_Lightning = 0x92,
	NPC_EnemyCritterHover = 0x93,
	NPC_ProjectileCritter = 0x94,
	NPC_MovingBlockHorizontal = 0x95,
	NPC_Quote = 0x96,
	NPC_BlueRobot = 0x97,
	NPC_ShutterStuck = 0x98,
	NPC_EnemyGaudi = 0x99,
	NPC_EnemyGaudiDefeated = 0x9A,
	NPC_EnemyGaudiFlying = 0x9B,
	NPC_ProjectileGaudiFlying = 0x9C,
	NPC_MovingBlockVertical = 0x9D,
	NPC_ProjectileMonsterX = 0x9E,
	NPC_BossMonsterXDefeated = 0x9F,
	NPC_BossPoohBlack = 0xA0,
	NPC_ProjectilePoohBlack = 0xA1,
	NPC_PoohBlackDefeated = 0xA2,
	NPC_DrGero = 0xA3,
	NPC_NurseHasumi = 0xA4,
	NPC_CurlyCollapsed = 0xA5,
	NPC_Chaba = 0xA6,
	NPC_ProfessorBoosterFalling = 0xA7,
	NPC_Boulder = 0xA8,
	NPC_BossBalrogMissiles = 0xA9,
	NPC_ProjectileBalrogMissiles = 0xAA,
	NPC_EnemyFireWhirrr = 0xAB,
	NPC_ProjectileFireWhirrr = 0xAC,
	NPC_EnemyGaudiArmor = 0xAD,
	NPC_ProjectileGaudiArmor = 0xAE,
	NPC_EnemyGaudiEgg = 0xAF,
	NPC_EnemyBuyoBoyoBase = 0xB0,
	NPC_EnemyBuyoBuyo = 0xB1,
	NPC_ProjectileCoreSpinner = 0xB2,
	NPC_ProjectileCoreWisp = 0xB3,
	NPC_CurlyAI = 0xB4,
	NPC_CurlyAIMachineGun = 0xB5,
	NPC_CurlyAIPolarStar = 0xB6,
	NPC_CurlyAirTankBubble = 0xB7,
	NPC_ShutterLarge = 0xB8,
	NPC_ShutterSmall = 0xB9,
	NPC_LiftBlock = 0xBA,
	NPC_EnemyFuzzCore = 0xBB,
	NPC_FuzzEnemy = 0xBC,
	NPC_ProjectileHomingFlame = 0xBD,
	NPC_SurfaceRobot = 0xBE,
	NPC_WaterLevel = 0xBF,
	NPC_Scooter = 0xC0,
	NPC_ScooterPieces = 0xC1,
	NPC_BlueRobotPieces = 0xC2,
	NPC_GrateMouth = 0xC3,
	NPC_MotionWall = 0xC4,
	NPC_EnemyPorcupineFish = 0xC5,
	NPC_ProjectileIronhead = 0xC6,
	NPC_UnderwaterCurrent = 0xC7,
	NPC_EnemyDragonZombie = 0xC8,
	NPC_DragonZombieDead = 0xC9,
	NPC_ProjectileDragonZombie = 0xCA,
	NPC_EnemyCritterHoppingAqua = 0xCB,
	NPC_FallingSpikeSmall = 0xCC,
	NPC_FallingSpikeLarge = 0xCD,
	NPC_EnemyCounterBomb = 0xCE,
	NPC_BalloonCountdown = 0xCF,
	NPC_EnemyBasu2 = 0xD0,
	NPC_ProjectileBasu2 = 0xD1,
	NPC_EnemyBeetleFollow2 = 0xD2,
	NPC_Spikes = 0xD3,
	NPC_SkyDragon = 0xD4,
	NPC_EnemyNightSpirit = 0xD5,
	NPC_ProjectileNightSpirit = 0xD6,
	NPC_EnemySandcrocwhite = 0xD7,
	NPC_DebugCat = 0xD8,
	NPC_Itoh = 0xD9,
	NPC_ProjectileCoreLargeEnergyBall = 0xDA,
	NPC_GeneratorSmokeUnderwaterCurrent = 0xDB,
	NPC_ShovelBrigade = 0xDC,
	NPC_ShovelBrigadeWalking = 0xDD,
	NPC_PrisonBars = 0xDE,
	NPC_Momorin = 0xDF,
	NPC_Chie = 0xE0,
	NPC_Megane = 0xE1,
	NPC_KanpachiStanding = 0xE2,
	NPC_Bucket = 0xE3,
	NPC_DrollGuard = 0xE4,
	NPC_RedFlowersSprouts = 0xE5,
	NPC_RedFlowersBlooming = 0xE6,
	NPC_Rocket = 0xE7,
	NPC_EnemyOrangeBell = 0xE8,
	NPC_EnemyOrangeBellBat = 0xE9,
	NPC_RedFlowerSpicked = 0xEA,
	NPC_EnemyMidorin = 0xEB,
	NPC_EnemyGunfish = 0xEC,
	NPC_ProjectileGunfish = 0xED,
	NPC_EnemyPressKiller = 0xEE,
	NPC_CageBars = 0xEF,
	NPC_MimigaJailed = 0xF0,
	NPC_EnemyCritterHoppingRed = 0xF1,
	NPC_EnemyBatRed = 0xF2,
	NPC_GeneratorBatRed = 0xF3,
	NPC_AcidDrop = 0xF4,
	NPC_GeneratorAcidDrop = 0xF5,
	NPC_EnemyPressProximity = 0xF6,
	NPC_BossMisery = 0xF7,
	NPC_BossMiseryVanish = 0xF8,
	NPC_ProjectileMiseryEnergyShot = 0xF9,
	NPC_ProjectileMiseryLightningBall = 0xFA,
	NPC_ProjectileMiseryLightning = 0xFB,
	NPC_ProjectileMiseryBlackOrbitingRings = 0xFC,
	NPC_EnergyCapsule = 0xFD,
	NPC_Helicopter = 0xFE,
	NPC_HelicopterBlades = 0xFF,
	NPC_DoctorCrownedFacingAway = 0x100,
	NPC_RedCrystal = 0x101,
	NPC_MimigaSleeping = 0x102,
	NPC_CurlyCarriedAwayUnconscious = 0x103,
	NPC_ShovelBrigadeCaged = 0x104,
	NPC_ChieCaged = 0x105,
	NPC_ChacoCaged = 0x106,
	NPC_BossDoctor = 0x107,
	NPC_ProjectileDoctorRedWave = 0x108,
	NPC_ProjectileDoctorRedBallFastVanish = 0x109,
	NPC_ProjectileDoctorRedBallSlowVanish = 0x10A,
	NPC_BossMuscleDoctor = 0x10B,
	NPC_EnemyIgor = 0x10C,
	NPC_EnemyBatRedEnergy = 0x10D,
	NPC_RedEnergy = 0x10E,
	NPC_UnderwaterBlock = 0x10F,
	NPC_GeneratorUnderwaterBlock = 0x110,
	NPC_ProjectileDroll = 0x111,
	NPC_EnemyDroll = 0x112,
	NPC_PuppyWithItems = 0x113,
	NPC_BossRedDemon = 0x114,
	NPC_ProjectileRedDemon = 0x115,
	NPC_LittleFamily = 0x116,
	NPC_FallingBlockLarge = 0x117,
	NPC_SueTeleportedInByMisery = 0x118,
	NPC_DoctorRedEnergyForm = 0x119,
	NPC_MiniUndeadCoreFloatsForward = 0x11A,
	NPC_EnemyMiseryTransformed = 0x11B,
	NPC_EnemySueTransformed = 0x11C,
	NPC_ProjectileUndeadCoreOrangeSpiralShot = 0x11D,
	NPC_OrangeDot = 0x11E,
	NPC_OrangeSmoke = 0x11F,
	NPC_ProjectileUndeadCoreGlowingRockThing = 0x120,
	NPC_EnemyCritterHoppingOrange = 0x121,
	NPC_EnemyBatOrange = 0x122,
	NPC_MiniUndeadCoreStationary = 0x123,
	NPC_Quake = 0x124,
	NPC_ProjectileUndeadCoreLargeEnergyBall = 0x125,
	NPC_QuakeGeneratorFallingBlocks = 0x126,
	NPC_Cloud = 0x127,
	NPC_GeneratorCloud = 0x128,
	NPC_SueOnSkyDragon = 0x129,
	NPC_DoctorWithoutCrown = 0x12A,
	NPC_BalrogMiseryInBubble = 0x12B,
	NPC_DemonCrown = 0x12C,
	NPC_EnemyFishMissileOrange = 0x12D,
	NPC_CameraFocusMarker = 0x12E,
	NPC_CurlysMachineGun = 0x12F,
	NPC_GaudiSitting = 0x130,
	NPC_PuppySmall = 0x131,
	NPC_BalrogNurse = 0x132,
	NPC_SantaCaged = 0x133,
	NPC_EnemyStumpy = 0x134,
	NPC_EnemyBute = 0x135,
	NPC_EnemyButeSword = 0x136,
	NPC_EnemyButeArcher = 0x137,
	NPC_ProjectileButeArcher = 0x138,
	NPC_BossMaPignon = 0x139,
	NPC_FallingIndestructible = 0x13A,
	NPC_EnemyHoppingDisappears = 0x13B,
	NPC_EnemyButeDefeated = 0x13C,
	NPC_EnemyMesa = 0x13D,
	NPC_EnemyMesaDefeated = 0x13E,
	NPC_ProjectileMesaBlock = 0x13F,
	NPC_CurlyCarriedShooting = 0x140,
	NPC_ProjectileCurlyNemesisBulletSpawner = 0x141,
	NPC_EnemyDeleet = 0x142,
	NPC_EnemyButeGenerator = 0x143,
	NPC_GeneratorBute = 0x144,
	NPC_ProjectileHeavyPress = 0x145,
	NPC_ItohSueTurningHuman = 0x146,
	NPC_ItohSueAhChoo = 0x147,
	NPC_Transmogrifier = 0x148,
	NPC_BuildingFan = 0x149,
	NPC_EnemyRolling = 0x14A,
	NPC_ProjectileBallosBone = 0x14B,
	NPC_ProjectileBallosShockwave = 0x14C,
	NPC_ProjectileBallosLightning = 0x14D,
	NPC_Sweat = 0x14E,
	NPC_Ikachan = 0x14F,
	NPC_GeneratorIkachan = 0x150,
	NPC_Numahachi = 0x151,
	NPC_EnemyGreenDevil = 0x152,
	NPC_GeneratorGreenDevil = 0x153,
	NPC_BossBallos = 0x154,
	NPC_Ballos1Head = 0x155,
	NPC_EnemyBallos3Eyeball = 0x156,
	NPC_Ballos2Cutscene = 0x157,
	NPC_Ballos2Eyes = 0x158,
	NPC_ProjectileBallosSkull = 0x159,
	NPC_Ballos4OrbitingPlatform = 0x15A,
	NPC_EnemyHoppy = 0x15B,
	NPC_BallosSpikesRising = 0x15C,
	NPC_Statue = 0x15D,
	NPC_EnemyButeArcherRed = 0x15E,
	NPC_StatueCanShoot = 0x15F,
	NPC_KingSword = 0x160,
	NPC_EnemyButeSwordRed = 0x161,
	NPC_InvisibleDeathTrapWall = 0x162,
	NPC_BalrogCrashingThroughWall = 0x163,
	NPC_BalrogRescue = 0x164,
	NPC_PuppyGhost = 0x165,
	NPC_MiseryWind = 0x166,
	NPC_GeneratorWaterDrop = 0x167,
	NPC_ThankYou = 0x168,
};
