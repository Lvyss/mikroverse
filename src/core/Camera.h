#pragma once
#include <glad/glad.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

// ── Batas ruangan Lab ─────────────────────────────────────
constexpr float LAB_MIN_X     = -13.0f;   // sesuai room width 28 -> hw=14, margin 1
constexpr float LAB_MAX_X     =  13.0f;
constexpr float LAB_MIN_Z     = -17.0f;   // sesuai room depth 36 -> hd=18, margin 1
constexpr float LAB_MAX_Z     =  17.0f;
constexpr float FLOOR_HEIGHT  =   0.0f;
constexpr float CEIL_HEIGHT   =   8.0f;
constexpr float PLAYER_HEIGHT =   1.75f;
constexpr float PLAYER_RADIUS =   0.4f;   // collision radius horizontal

enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw              = -90.0f;
    float Pitch            =   0.0f;
    float Zoom             =  45.0f;
    float MovementSpeed    =   5.0f;
    float MouseSensitivity =   0.1f;
    float SprintMultiplier =   2.0f;  // shift = sprint di fly mode

    glm::vec3 Velocity  = glm::vec3(0.0f);
    bool isFlyMode  = false;   // default: WALK (gravitasi aktif)
    bool isOnGround = false;
    bool isSprinting = false;

    Camera(glm::vec3 position = glm::vec3(0.0f, FLOOR_HEIGHT + PLAYER_HEIGHT, 14.0f))
        : Position(position), WorldUp(glm::vec3(0.0f, 1.0f, 0.0f))
    { updateCameraVectors(); }

    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(Camera_Movement dir, float dt) {
        float speed = MovementSpeed * dt;
        if (isFlyMode && isSprinting) speed *= SprintMultiplier;

        // Di walk mode, gerak hanya di bidang horizontal (ignore pitch)
        glm::vec3 flatFront = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
        glm::vec3 flatRight = glm::normalize(glm::vec3(Right.x, 0.0f, Right.z));

        glm::vec3 move(0.0f);

        if (!isFlyMode) {
            // ── WALK MODE: gerak flat ─────────────────────
            if (dir == FORWARD)  move += flatFront * speed;
            if (dir == BACKWARD) move -= flatFront * speed;
            if (dir == LEFT)     move -= flatRight * speed;
            if (dir == RIGHT)    move += flatRight * speed;
            // UP/DOWN diabaikan di walk mode (jump pakai Jump())

            // Apply movement
            Position += move;

            // Collision dinding — clamp dengan margin PLAYER_RADIUS
            Position.x = glm::clamp(Position.x,
                LAB_MIN_X + PLAYER_RADIUS,
                LAB_MAX_X - PLAYER_RADIUS);
            Position.z = glm::clamp(Position.z,
                LAB_MIN_Z + PLAYER_RADIUS,
                LAB_MAX_Z - PLAYER_RADIUS);

        } else {
            // ── FLY MODE: gerak 6 arah bebas ─────────────
            if (dir == FORWARD)  move += Front    * speed;
            if (dir == BACKWARD) move -= Front    * speed;
            if (dir == LEFT)     move -= Right    * speed;
            if (dir == RIGHT)    move += Right    * speed;
            if (dir == UP)       move += WorldUp  * speed;
            if (dir == DOWN)     move -= WorldUp  * speed;

            Position += move;

            // Soft collision di fly mode (boleh ke langit-langit tapi tidak nembus)
            Position.x = glm::clamp(Position.x,
                LAB_MIN_X + PLAYER_RADIUS,
                LAB_MAX_X - PLAYER_RADIUS);
            Position.z = glm::clamp(Position.z,
                LAB_MIN_Z + PLAYER_RADIUS,
                LAB_MAX_Z - PLAYER_RADIUS);
            Position.y = glm::clamp(Position.y,
                FLOOR_HEIGHT + 0.3f,           // tidak nembus lantai
                CEIL_HEIGHT  - 0.3f);           // tidak nembus langit-langit
        }
    }

    void UpdatePhysics(float dt) {
    if (isFlyMode) {
        // FLY MODE: gravitasi MATI, gerak bebas 6 arah
        // Jangan geser posisi sama sekali di sini (gerak udah di ProcessKeyboard)
        // Cuma clamp biar ga tembus tembok/lantai/langit-langit
        
        // Soft clamp biar ga tembus batas ruangan
        Position.x = glm::clamp(Position.x,
            LAB_MIN_X + PLAYER_RADIUS,
            LAB_MAX_X - PLAYER_RADIUS);
        Position.z = glm::clamp(Position.z,
            LAB_MIN_Z + PLAYER_RADIUS,
            LAB_MAX_Z - PLAYER_RADIUS);
        Position.y = glm::clamp(Position.y,
            FLOOR_HEIGHT + 0.2f,
            CEIL_HEIGHT - 0.2f);
        
        // Reset velocity (optional, biar ga bawa momentum dari walk mode)
        Velocity = glm::vec3(0.0f);
        return;  // LANGSUNG KELUAR, ga kena gravitasi
    }
        // ── WALK MODE PHYSICS ─────────────────────────────

        // Gravitasi
        const float GRAVITY     = 18.0f;   // lebih natural dari 9.8
        const float TERM_VEL_Y  = -20.0f;  // terminal velocity turun

        Velocity.y -= GRAVITY * dt;
        if (Velocity.y < TERM_VEL_Y) Velocity.y = TERM_VEL_Y;

        // Apply velocity
        Position += Velocity * dt;

        // ── Collision lantai ──────────────────────────────
        float groundY = FLOOR_HEIGHT + PLAYER_HEIGHT;
        if (Position.y <= groundY) {
            Position.y = groundY;
            Velocity.y = 0.0f;
            isOnGround = true;
        } else {
            isOnGround = false;
        }

        // ── Collision langit-langit ───────────────────────
        float ceilY = CEIL_HEIGHT - 0.3f;
        if (Position.y > ceilY) {
            Position.y = ceilY;
            Velocity.y = 0.0f;  // kepala nabrak langit-langit
        }

        // ── Collision dinding ─────────────────────────────
        Position.x = glm::clamp(Position.x,
            LAB_MIN_X + PLAYER_RADIUS,
            LAB_MAX_X - PLAYER_RADIUS);
        Position.z = glm::clamp(Position.z,
            LAB_MIN_Z + PLAYER_RADIUS,
            LAB_MAX_Z - PLAYER_RADIUS);

        // ── Friction horizontal (biar tidak licin) ────────
        // Hanya di ground
        if (isOnGround) {
            Velocity.x *= 0.80f;
            Velocity.z *= 0.80f;
        }
    }

    void Jump() {
        if (!isFlyMode && isOnGround) {
            Velocity.y = 7.0f;   // kekuatan lompat
            isOnGround = false;
        }
    }

    // Toggle walk <-> fly, reset velocity saat switch
    void ToggleMode() {
        isFlyMode  = !isFlyMode;
        Velocity   = glm::vec3(0.0f);
        if (isFlyMode) {
            // Saat masuk fly, naik sedikit biar tidak stuck di lantai
            // (tidak perlu, position sudah valid)
        } else {
            // Saat balik ke walk, pastikan tidak melayang terlalu tinggi
            // Biarkan gravitasi yang tarik turun secara natural
        }
    }

    void SetSprinting(bool sprint) { isSprinting = sprint; }

    void ProcessMouseMovement(float xoff, float yoff, bool constrainPitch = true) {
        Yaw   += xoff * MouseSensitivity;
        Pitch += yoff * MouseSensitivity;
        if (constrainPitch) Pitch = glm::clamp(Pitch, -89.0f, 89.0f);
        updateCameraVectors();
    }

    void ProcessMouseScroll(float yoff) {
        Zoom = glm::clamp(Zoom - yoff, 1.0f, 90.0f);
    }

private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cosf(glm::radians(Yaw)) * cosf(glm::radians(Pitch));
        front.y = sinf(glm::radians(Pitch));
        front.z = sinf(glm::radians(Yaw)) * cosf(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};