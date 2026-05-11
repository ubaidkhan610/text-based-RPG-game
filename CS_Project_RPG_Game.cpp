#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cctype> // for toupper

// ==============================================================================
// FORWARD DECLARATIONS
// ==============================================================================
class Character;
class Weapon;
   
// ==============================================================================
// CLASS DECLARATIONS
// ==============================================================================

// --- ITEM SYSTEM ---
class Item {
protected:
    std::string name;
    int type; // 1 = Potion, 2 = Weapon

public:
    Item(std::string n, int t) : name(n), type(t) {}
    virtual ~Item() = default;

    virtual void use(Character& target) = 0; // Pure virtual

    std::string getName() const { return name; }
    int getType() const { return type; }
    virtual int getEffectValue() const = 0;
};

class HealthPotion : public Item {
private:
    int healAmount;
public:
    HealthPotion(std::string n, int heal) : Item(n, 1), healAmount(heal) {}
    void use(Character& target) override;
    int getEffectValue() const override { return healAmount; }
};

class Weapon : public Item {
private:
    int damageBonus;
public:
    Weapon(std::string n, int dmg) : Item(n, 2), damageBonus(dmg) {}
    void use(Character& target) override;
    int getEffectValue() const override { return damageBonus; }
};


// --- CHARACTER SYSTEM ---
class Character {
protected:
    std::string name;
    int health, maxHealth, attackPower, defense, level, experience;
    int mana, maxMana;
    bool stunned;
    std::vector<Item*> inventory;
    Weapon* equippedWeapon;

public:
    Character(std::string n, int hp, int atk, int def, int mp = 0);
    virtual ~Character();

    virtual void attack(Character& target) = 0;
    virtual void useSpecialAbility(Character& target) = 0;

    std::string getName() const { return name; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    int getBaseAttackPower() const { return attackPower; }   // without weapon bonus
    int getAttackPower() const;                              // with weapon bonus
    int getDefense() const { return defense; }
    int getLevel() const { return level; }
    int getExperience() const { return experience; }
    int getMana() const { return mana; }
    bool isStunned() const { return stunned; }
    void setStunned(bool s) { stunned = s; }

    void heal(int amount);
    virtual void takeDamage(int amount);
    void addExperience(int amount);
    void levelUp();
    
    void addItem(Item* item);
    void equipWeapon(Weapon* w);
    void showInventory();
    bool useItemFromInventory(int index);
    
    std::vector<Item*>& getInventory() { return inventory; }
    Weapon* getEquippedWeapon() { return equippedWeapon; }

    void setStats(int hp, int mhp, int atk, int def, int lvl, int xp, int mp);
};

// --- PLAYER CLASSES ---
class Warrior : public Character {
private:
    int shieldBashCooldown;
public:
    Warrior(std::string n) : Character(n, 150, 20, 10), shieldBashCooldown(0) {}
    void attack(Character& target) override;
    void useSpecialAbility(Character& target) override;
    void decrementCooldown() { if (shieldBashCooldown > 0) shieldBashCooldown--; }
};

class Mage : public Character {
public:
    Mage(std::string n) : Character(n, 90, 15, 5, 50) {}
    void attack(Character& target) override;
    void useSpecialAbility(Character& target) override;
};

class Rogue : public Character {
public:
    Rogue(std::string n) : Character(n, 110, 25, 7) {}
    void attack(Character& target) override;
    void useSpecialAbility(Character& target) override;
};


// --- ENEMY SYSTEM ---
class Enemy : public Character {
protected:
    int goldDrop;
    int xpDrop;
public:
    Enemy(std::string n, int hp, int atk, int def, int gold, int xp)
        : Character(n, hp, atk, def), goldDrop(gold), xpDrop(xp) {}
    
    int getGoldDrop() const { return goldDrop; }
    int getXpDrop() const { return xpDrop; }
    
    void attack(Character& target) override;
    void useSpecialAbility(Character& target) override { /* basic enemies do nothing */ }
};

class Skeleton : public Enemy {
public:
    Skeleton() : Enemy("Skeleton", 50, 15, 5, 10, 40) {}
};

class Goblin : public Enemy {
public:
    Goblin() : Enemy("Goblin", 60, 20, 2, 15, 60) {}
};

class DragonBoss : public Enemy {
public:
    DragonBoss() : Enemy("Dragon Boss", 300, 40, 15, 100, 500) {}
    void attack(Character& target) override; 
};


// --- ROOM SYSTEM ---
class Room {
private:
    std::string name;
    std::string description;
    Enemy* enemy;
    Item* item;
    int exits[4]; 

public:
    Room(std::string n, std::string desc, Enemy* e = nullptr, Item* i = nullptr);
    ~Room();

    void setExits(int n, int s, int e, int w);
    int getExit(int direction) const { return exits[direction]; }
    
    std::string getName() const { return name; }
    std::string getDescription() const { return description; }
    Enemy* getEnemy() const { return enemy; }
    Item* getItem() const { return item; }
    
    // Transfer or delete ownership carefully:
    void clearEnemy()  { enemy = nullptr; }
    void clearItem()   { item = nullptr; }
    void deleteEnemy() { delete enemy; enemy = nullptr; }
    void deleteItem()  { delete item; item = nullptr; }
    
    void look() const;
};


// --- GAME ENGINE ---
class Game {
private:
    std::vector<Room*> dungeon;
    Character* player;
    int currentRoomIndex;
    bool isRunning;

    void createDungeonLayout();   // only creates rooms (no cleanup)
    void cleanUpDungeon();        // deletes all rooms
    void combatLoop(Enemy* enemy);
    void handleMovement();
    void saveGame();
    void loadGame();

public:
    Game();
    ~Game();
    
    void start();
    void mainMenu();
    void gameLoop();
};


// ==============================================================================
// METHOD IMPLEMENTATIONS
// ==============================================================================

// --- Item Methods ---
void HealthPotion::use(Character& target) {
    target.heal(healAmount);
    std::cout << "Used " << name << "! Restored " << healAmount << " HP.\n";
}

void Weapon::use(Character& target) {
    target.equipWeapon(this);
    std::cout << "Equipped " << name << "! Attack increased by " << damageBonus << ".\n";
}

// --- Character Methods ---
Character::Character(std::string n, int hp, int atk, int def, int mp)
    : name(n), health(hp), maxHealth(hp), attackPower(atk), defense(def),
      level(1), experience(0), mana(mp), maxMana(mp), stunned(false), equippedWeapon(nullptr) {}

Character::~Character() {
    for (Item* item : inventory) {
        delete item;
    }
}

int Character::getAttackPower() const {
    if (equippedWeapon) return attackPower + equippedWeapon->getEffectValue();
    return attackPower;// damage without weapon
}

void Character::takeDamage(int amount) {
    int actualDamage = std::max(0, amount - defense);
    health -= actualDamage;
    if (health < 0) health = 0;
    std::cout << name << " takes " << actualDamage << " damage! (HP: " << health << ")\n";
}

void Character::heal(int amount) {
    health = std::min(maxHealth, health + amount);
}

void Character::addExperience(int amount) {
    experience += amount;
    std::cout << name << " gained " << amount << " XP!\n";
    if (experience >= level * 100) {
        levelUp();
    }
}

void Character::levelUp() {
    experience -= level * 100;
    level++;
    maxHealth += 20;
    health = maxHealth;
    attackPower += 5;
    defense += 2;
    maxMana += 10;
    mana = maxMana;
    std::cout << "LEVEL UP! " << name << " is now level " << level << "!\n";
}

void Character::addItem(Item* item) {
    inventory.push_back(item);
    std::cout << "Picked up: " << item->getName() << "\n";
}

void Character::equipWeapon(Weapon* w) {
    equippedWeapon = w;
}

void Character::showInventory() {
    if (inventory.empty()) {
        std::cout << "Inventory is empty.\n";
        return;
    }
    for (size_t i = 0; i < inventory.size(); i++) {
        std::cout << i + 1 << ". " << inventory[i]->getName();
        if (inventory[i] == equippedWeapon) std::cout << " (Equipped)";
        std::cout << "\n";
    }
}

bool Character::useItemFromInventory(int index) {
    if (index < 0 || index >= (int)inventory.size()) return false;
    
    Item* item = inventory[index];
    item->use(*this);
    
    // Potions are consumed, weapons stay in inventory
    if (item->getType() == 1) { 
        inventory.erase(inventory.begin() + index);
        delete item;
    }
    return true;
}

void Character::setStats(int hp, int mhp, int atk, int def, int lvl, int xp, int mp) {
    health = hp; maxHealth = mhp; attackPower = atk; defense = def;
    level = lvl; experience = xp; mana = mp; maxMana = mp;
}

// --- PlayerClasses Methods ---
void Warrior::attack(Character& target) {
    std::cout << name << " swings their sword at " << target.getName() << "!\n";
    target.takeDamage(getAttackPower());
}

void Warrior::useSpecialAbility(Character& target) {
    if (shieldBashCooldown > 0) {
        std::cout << "Shield Bash is on cooldown! (" << shieldBashCooldown << " turns left)\n";
        return;
    }
    std::cout << name << " uses Shield Bash! " << target.getName() << " is stunned!\n";
    target.takeDamage(getAttackPower() / 2); 
    target.setStunned(true);
    shieldBashCooldown = 3;
}

void Mage::attack(Character& target) {
    std::cout << name << " fires a magic bolt at " << target.getName() << "!\n";
    target.takeDamage(getAttackPower());
}

void Mage::useSpecialAbility(Character& target) {
    if (mana >= 15) {
        std::cout << name << " casts Fireball!\n";
        mana -= 15;
        int dmg = getAttackPower() * 2;
        std::cout << target.getName() << " takes " << dmg << " piercing damage!\n";
        target.takeDamage(dmg + target.getDefense());  // ignores defence
    } else {
        std::cout << "Not enough mana!\n";
    }
}

void Rogue::attack(Character& target) {
    std::cout << name << " attacks " << target.getName() << " quickly!\n";
    int dmg = getAttackPower();
    if (rand() % 100 < 20) {   // 20% crit chance
        std::cout << "Critical Hit!\n";
        dmg = (int)(dmg * 1.5);
    }
    target.takeDamage(dmg);
}

void Rogue::useSpecialAbility(Character& target) {
    std::cout << name << " attempts a Backstab!\n";
    int dmg = getAttackPower();
    if (target.getHealth() > target.getMaxHealth() / 2) {
        std::cout << "Massive damage against healthy enemy!\n";
        dmg *= 2;
    }
    target.takeDamage(dmg);
}

// --- Enemy Methods ---
void Enemy::attack(Character& target) {
    std::cout << name << " attacks " << target.getName() << "!\n";
    target.takeDamage(getAttackPower());
}

void DragonBoss::attack(Character& target) {
    if (health < maxHealth / 2) {
        std::cout << name << " uses INFERNO BREATH!\n";
        target.takeDamage(getAttackPower() * 2);
    } else {
        std::cout << name << " claws at " << target.getName() << "!\n";
        target.takeDamage(getAttackPower());
    }
}

// --- Room Methods ---
Room::Room(std::string n, std::string desc, Enemy* e, Item* i) 
    : name(n), description(desc), enemy(e), item(i) {
    for(int j=0; j<4; ++j) exits[j] = -1;
}

Room::~Room() {
    delete enemy; 
    delete item; 
}

void Room::setExits(int n, int s, int e, int w) {
    exits[0] = n; exits[1] = s; exits[2] = e; exits[3] = w;
}

void Room::look() const {
    std::cout << "\n=== " << name << " ===\n" << description << "\n";
    if (enemy) std::cout << "Warning: A " << enemy->getName() << " is here!\n";
    if (item) std::cout << "You see a " << item->getName() << " on the ground.\n";
    
    std::cout << "Exits: ";
    if (exits[0] != -1) std::cout << "North ";
    if (exits[1] != -1) std::cout << "South ";
    if (exits[2] != -1) std::cout << "East ";
    if (exits[3] != -1) std::cout << "West ";
    std::cout << "\n";
}

// --- Game Engine Methods ---
Game::Game() : player(nullptr), currentRoomIndex(0), isRunning(true) {}

Game::~Game() {
    cleanUpDungeon();
    delete player;
}

void Game::cleanUpDungeon() {
    for (Room* r : dungeon) delete r;
    dungeon.clear();
}

void Game::createDungeonLayout() {
    // Build the five rooms with enemies/items
    Room* r0 = new Room("Starting Cell", "A damp, cold cell where you awoke.", nullptr, nullptr);
    Room* r1 = new Room("Dark Corridor", "A long hallway lit by torches.", new Goblin(), new HealthPotion("Minor Potion", 30));
    Room* r2 = new Room("Armory", "Racks of rusty weapons surround you.", new Skeleton(), new Weapon("Iron Sword", 15));
    Room* r3 = new Room("Treasure Room", "Glittering coins scatter the floor.", nullptr, new HealthPotion("Major Potion", 60));
    Room* r4 = new Room("Boss Chamber", "The air is heavy and hot. Death awaits.", new DragonBoss(), nullptr);

    r0->setExits(-1, -1, 1, -1);
    r1->setExits( 2, -1, 3,  0);
    r2->setExits(-1,  1,-1, -1);
    r3->setExits(-1, -1, 4,  1);
    r4->setExits(-1, -1,-1,  3);

    dungeon = {r0, r1, r2, r3, r4};  // vector 
    currentRoomIndex = 0;
}

void Game::mainMenu() {
    std::cout << "--- DUNGEON ESCAPE ---\n";
    std::cout << "1. New Game\n2. Load Game\n3. Quit\nChoice: ";
    int choice;
    std::cin >> choice;

    if (choice == 1) {
        // Clean up previous game if any
        delete player;
        player = nullptr;
        cleanUpDungeon();

        std::cout << "Choose Class (1. Warrior, 2. Mage, 3. Rogue): ";
        int cls; std::cin >> cls;
        std::cout << "Enter Name: ";
        std::string name; std::cin >> name;
        
        if (cls == 1) player = new Warrior(name);
        else if (cls == 2) player = new Mage(name);
        else player = new Rogue(name);
        
        createDungeonLayout();
        gameLoop();
    } else if (choice == 2) {
        loadGame();
        if (player) gameLoop();
    } else {
        isRunning = false;
    }
}

void Game::start() {
    while (isRunning) {
        mainMenu();
    }
}

void Game::combatLoop(Enemy* enemy) {
    std::cout << "\nCOMBAT INITIATED! " << enemy->getName() << " attacks!\n";
    while (player->getHealth() > 0 && enemy->getHealth() > 0) {
        if (Warrior* w = dynamic_cast<Warrior*>(player)) w->decrementCooldown();
        
        std::cout << "\n[HP: " << player->getHealth() << "/" << player->getMaxHealth() << "] Turn:\n";
        std::cout << "1. Attack  2. Special  3. Item  4. Run\nChoice: ";
        int choice;
        std::cin >> choice;

        bool playerActed = true; // did player perform an action?
        if (choice == 1) {
            player->attack(*enemy);
        } else if (choice == 2) {
            player->useSpecialAbility(*enemy);
        } else if (choice == 3) {
            player->showInventory();
            std::cout << "Select item (0 to cancel): ";
            int itemIdx; std::cin >> itemIdx;
            if (itemIdx > 0)
                player->useItemFromInventory(itemIdx - 1);
            // even if cancel, still counts as a turn
        } else if (choice == 4) {
            if (rand() % 100 < 50) {
                std::cout << "Fled successfully!\n";
                return;
            } else {
                std::cout << "Failed to run!\n";
            }
        } else {
            // invalid input, retry the same turn
            continue;
        }

        // Check if enemy died
        if (enemy->getHealth() <= 0) {
            std::cout << "Defeated " << enemy->getName() << "!\n";
            player->addExperience(enemy->getXpDrop());
            // Delete enemy and remove from room
            Room* room = dungeon[currentRoomIndex];
            room->deleteEnemy();   // deletes the enemy object and sets pointer to null
            if (enemy->getName() == "Dragon Boss") {
                std::cout << "\nYOU DEFEATED THE BOSS AND ESCAPED THE DUNGEON! YOU WIN!\n";
                isRunning = false;
            }
            break;
        }

        // Enemy's turn (unless stunned)
        if (!enemy->isStunned()) {
            enemy->attack(*player);
        } else {
            std::cout << enemy->getName() << " is stunned and skips their turn!\n";
            enemy->setStunned(false);
        }
    }
    
    if (player->getHealth() <= 0) {
        std::cout << "You died... Game Over.\n";
        isRunning = false;
    }
}

void Game::handleMovement() {
    Room* r = dungeon[currentRoomIndex];
    std::cout << "Action (N, S, E, W to move | I for Inventory | P to pick up | V to save): ";
    char action;
    std::cin >> action;
    action = toupper(action);
    int dir = -1;
    if (action == 'N') dir = 0;
    else if (action == 'S') dir = 1;
    else if (action == 'E') dir = 2;
    else if (action == 'W') dir = 3;
    else if (action == 'I') { player->showInventory(); return; }
    else if (action == 'P') {
        if (r->getItem()) {
            player->addItem(r->getItem());
            r->clearItem(); // ownership transferred
        } else { std::cout << "Nothing to pick up.\n"; }
        return;
    }
    else if (action == 'V') { saveGame(); return; }
    else { std::cout << "Invalid action.\n"; return; }

    if (dir != -1) {
        int nextRoom = r->getExit(dir);
        if (nextRoom != -1) currentRoomIndex = nextRoom;
        else std::cout << "You can't go that way.\n";
    }
}

void Game::gameLoop() {
    while (isRunning && player->getHealth() > 0) {
        Room* r = dungeon[currentRoomIndex];
        r->look();
        
        if (r->getEnemy()) {
            combatLoop(r->getEnemy());
            if (!isRunning) break; // player died or won
        }
        
        if (isRunning) handleMovement();
    }
}

void Game::saveGame() {
    std::ofstream out("savegame.txt");
    if (!out) return;
    
    int pType = 0;
    if (dynamic_cast<Warrior*>(player)) pType = 1;
    else if (dynamic_cast<Mage*>(player)) pType = 2;
    else if (dynamic_cast<Rogue*>(player)) pType = 3;

    out << pType << "\n" << player->getName() << "\n";
    // Save base attack (without weapon) – weapon bonus is applied when reloading
    out << player->getHealth() << " " << player->getMaxHealth() << " " 
        << player->getBaseAttackPower() << " " << player->getDefense() << " " 
        << player->getLevel() << " " << player->getExperience() << " " 
        << player->getMana() << "\n";
    
    auto& inv = player->getInventory();
    out << inv.size() << "\n";
    for (Item* item : inv) {
        out << item->getType() << " " << item->getEffectValue() << " " << item->getName() << "\n";
    }
    
    // Save equipped weapon index in inventory (or -1 if none)
    int eqIdx = -1;
    if (player->getEquippedWeapon()) {
        for (size_t i = 0; i < inv.size(); ++i)
            if (inv[i] == player->getEquippedWeapon()) { eqIdx = i; break; }
    }
    out << eqIdx << "\n";

    out << currentRoomIndex << "\n";
    for (Room* r : dungeon) {
        out << (r->getEnemy() != nullptr) << " " << (r->getItem() != nullptr) << "\n";
    }
    std::cout << "Game saved successfully!\n";
}

void Game::loadGame() {
    std::ifstream in("savegame.txt");
    if (!in) { std::cout << "No save found.\n"; return; }
    
    // Clean up current game (but keep this loading attempt isolated)
    delete player; player = nullptr;
    cleanUpDungeon();
    
    int pType; in >> pType;
    std::string name; in >> name;
    
    if (pType == 1) player = new Warrior(name);
    else if (pType == 2) player = new Mage(name);
    else if (pType == 3) player = new Rogue(name);
    else { std::cout << "Corrupt save.\n"; return; }

    int hp, mhp, atk, def, lvl, xp, mp;
    in >> hp >> mhp >> atk >> def >> lvl >> xp >> mp;
    player->setStats(hp, mhp, atk, def, lvl, xp, mp);

    int invSize; in >> invSize;
    std::vector<Item*> loadedItems; // keep track for equipping
    for (int i=0; i<invSize; i++) {
        int t, val; std::string n;
        in >> t >> val;
        std::getline(in >> std::ws, n);
        Item* item = nullptr;
        if (t == 1) item = new HealthPotion(n, val);
        else if (t == 2) item = new Weapon(n, val);
        if (item) {
            player->addItem(item);
            loadedItems.push_back(item);
        }
    }

    int eqIdx; in >> eqIdx;
    if (eqIdx >= 0 && eqIdx < (int)loadedItems.size()) {
        player->equipWeapon(dynamic_cast<Weapon*>(loadedItems[eqIdx]));
    }

    // Create fresh dungeon layout
    createDungeonLayout();

    in >> currentRoomIndex;
    
    // Adjust rooms according to save: delete enemies/items if flagged false
    for (size_t i = 0; i < dungeon.size(); i++) {
        bool hasE, hasI;
        in >> hasE >> hasI;
        if (!hasE && dungeon[i]->getEnemy()) {
            dungeon[i]->deleteEnemy(); // deletes enemy and sets ptr null
        }
        if (!hasI && dungeon[i]->getItem()) {
            dungeon[i]->deleteItem();  // deletes item and sets ptr null
        }
    }
    std::cout << "Game loaded!\n";
}

// ==============================================================================
// MAIN FUNCTION
// ==============================================================================
int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    Game game;
    game.start();
    return 0;
}