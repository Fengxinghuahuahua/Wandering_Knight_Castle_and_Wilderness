
#include <btBulletDynamicsCommon.h>
#include <vector>
#include <iostream>
#include <model_animation.h>
#include <utility>
#include <functional>

class Knight;

class CollisionListener : public btCollisionWorld::ContactResultCallback {
private:
    btRigidBody* bodyA_;
    btRigidBody* bodyB_;
    btVector3 collisionNormal_;  // 保存碰撞法线
    bool collisionDetected_ = false;

public:
    CollisionListener(btRigidBody* bodyA, btRigidBody* bodyB) : bodyA_(bodyA), bodyB_(bodyB) {}

    bool isCollisionDetected() const {
        return collisionDetected_;
    }

    btVector3 getCollisionNormal() const {
        return collisionNormal_;
    }

    virtual btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0, int partId0, int index0,
                                     const btCollisionObjectWrapper* colObj1, int partId1, int index1) override {
        const btCollisionObject* objA = colObj0->getCollisionObject();
        const btCollisionObject* objB = colObj1->getCollisionObject();

        if ((objA == bodyA_ && objB == bodyB_) || (objA == bodyB_ && objB == bodyA_)) {
            collisionDetected_ = true;
            collisionNormal_ = cp.m_normalWorldOnB;  // 获取碰撞法线
        }

        return 0;
    }
};


enum RIGIBODY_TYPE {BOX = 0, CYLINDER, CAPSULE, CONVEX_HULL};

class PhysicsWorld {
public:
    using CollisionHandler = std::function<void(btRigidBody*, btRigidBody*, const btVector3&, Knight*)>;
    PhysicsWorld() {
        // 初始化 Bullet 物理世界
        broadphase = new btDbvtBroadphase();
        collisionConfiguration = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher(collisionConfiguration);
        solver = new btSequentialImpulseConstraintSolver();
        dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
        dynamicsWorld->setGravity(btVector3(0, -9.81, 0)); // 设置重力
    }

    ~PhysicsWorld() {
        // 清理资源
        for (btCollisionShape* shape : collisionShapes) {
            delete shape;
        }
        for (btRigidBody* body : rigidBodies) {
            dynamicsWorld->removeRigidBody(body);
            delete body->getMotionState();
            delete body;
        }
        delete dynamicsWorld;
        delete solver;
        delete dispatcher;
        delete collisionConfiguration;
        delete broadphase;
    }
    void stepSimulation(float deltaTime, Knight* app) {
        // 更新物理世界
        dynamicsWorld->stepSimulation(deltaTime);
        updateCollision(app);
    }

    // 创建碰撞监听器
    void registerCollision(btRigidBody* bodyA, btRigidBody* bodyB, CollisionHandler handler) {
        registeredCollisions.push_back({{bodyA, bodyB}, handler});
    }

    // 添加模型的碰撞体
    btRigidBody* addModelCollision(Model* model, int add_mode, const glm::mat4& transformMatrix, bool noContactResponse = false) {
        btCollisionShape* collisionShape = createCollisionShapeFromModel(model, add_mode, transformMatrix);
        btVector3 position = calculateModelCenter(model, transformMatrix);  // 使用变换后的模型中心位置
        std::cout << "Position: " << position.getX() << " " << position.getY() << " " << position.getZ() << std::endl;
        return createRigidBody(collisionShape, position, 0.0f, noContactResponse);
    }

    // 创建包围盒碰撞体
    btCollisionShape* createBoundingBoxCollisionShape(const glm::vec3& min, const glm::vec3& max) {
        btVector3 halfExtents((max.x - min.x) / 2.0f, (max.y - min.y) / 2.0f, (max.z - min.z) / 2.0f);
        return new btBoxShape(halfExtents);
    }

        // 创建刚体
    btRigidBody* createRigidBody(btCollisionShape* shape, const btVector3& position, float mass, bool noContactResponse) {
        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(position);

        btVector3 localInertia(0, 0, 0);
        if (mass > 0.0f) {
            shape->calculateLocalInertia(mass, localInertia);
        }

        btDefaultMotionState* motionState = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionState, shape, localInertia);
        btRigidBody* body = new btRigidBody(rigidBodyCI);

        if (noContactResponse) {
            body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
        }

        dynamicsWorld->addRigidBody(body);
        return body;
    }



private:
    btBroadphaseInterface* broadphase;
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btSequentialImpulseConstraintSolver* solver;
    btDiscreteDynamicsWorld* dynamicsWorld;

    std::vector<btCollisionShape*> collisionShapes; // 保存所有碰撞形状
    std::vector<btRigidBody*> rigidBodies;         // 保存所有刚体
    std::vector<std::pair<std::pair<btRigidBody*, btRigidBody*>, CollisionHandler>> registeredCollisions;


    // 每一帧更新时，检测碰撞并处理
    // 更新碰撞监听
    void updateCollision(Knight* app) {
        for (auto& [bodyPair, handler] : registeredCollisions) {
            btRigidBody* bodyA = bodyPair.first;
            btRigidBody* bodyB = bodyPair.second;

            // 创建监听器
            CollisionListener listener(bodyA, bodyB);
            dynamicsWorld->contactTest(bodyA, listener);

            // 如果监听器检测到碰撞，调用对应的处理逻辑
            if (listener.isCollisionDetected()) {
                btVector3 collisionNormal = listener.getCollisionNormal();
                handler(bodyA, bodyB, collisionNormal, app);
            }
        }
    }

        // 根据模型生成对应的碰撞体
    btCollisionShape* createCollisionShapeFromModel(Model* model, int add_mode, const glm::mat4& transformMatrix) {
        if (model->meshes.empty()) return nullptr;

        glm::vec3 globalMin = glm::vec3(FLT_MAX);
        glm::vec3 globalMax = glm::vec3(-FLT_MAX);

        // 遍历所有网格，计算变换后的全局包围盒
        for (Mesh& mesh : model->meshes) {
            glm::vec3 min, max;
            mesh.GetBoundingBox(min, max, transformMatrix);
            globalMin = glm::min(globalMin, min);
            globalMax = glm::max(globalMax, max);
            std::cout << "Global Min: " << globalMin.x << ", " << globalMin.y << ", " << globalMin.z << std::endl;
            std::cout << "Global Max: " << globalMax.x << ", " << globalMax.y << ", " << globalMax.z << std::endl;
        }

        std::cout << "--------------------" << endl;

        // 根据全局包围盒创建对应的碰撞体
        if (add_mode == RIGIBODY_TYPE::BOX) {
            return createBoundingBoxCollisionShape(globalMin, globalMax);
        } else if (add_mode == RIGIBODY_TYPE::CYLINDER) {
            return createCylinderCollisionShape(globalMin, globalMax);
        } else if (add_mode == RIGIBODY_TYPE::CAPSULE) {
            return createCapsuleCollisionShape(globalMin, globalMax);
        } else if (add_mode == RIGIBODY_TYPE::CONVEX_HULL) {
            return createConvexHullCollisionShape(model, transformMatrix);
        }
        return nullptr;
    }

    // 创建合并后的圆柱碰撞体
    btCollisionShape* createCylinderCollisionShape(const glm::vec3& min, const glm::vec3& max) {
        float radius = (max.x - min.x) / 2.0f;
        float height = max.y - min.y;
        return new btCylinderShape(btVector3(radius, height / 2.0f, radius));
    }

    // 创建合并后的胶囊碰撞体
    btCollisionShape* createCapsuleCollisionShape(const glm::vec3& min, const glm::vec3& max) {
        float radius = (max.x - min.x) / 2.0f;
        float height = max.y - min.y;
        return new btCapsuleShape(radius, height);
    }


    btCollisionShape* createConvexHullCollisionShape(Model* model, const glm::mat4& transformMatrix) {
        if (model->meshes.empty()) return nullptr;

        btConvexHullShape* convexShape = new btConvexHullShape();

        // 遍历所有网格
        for (Mesh& mesh : model->meshes) {
            // 遍历网格的每个顶点
            for (const Vertex& vertex : mesh.vertices) {
                // 应用变换矩阵
                glm::vec4 transformedPos = transformMatrix * glm::vec4(vertex.Position, 1.0f);
                glm::vec3 worldPos = glm::vec3(transformedPos);

                // 将变换后的顶点添加到凸包形状中
                convexShape->addPoint(btVector3(worldPos.x, worldPos.y, worldPos.z));
            }
        }

        return convexShape;
    }


    
    // 计算模型的中心位置
    btVector3 calculateModelCenter(Model* model, const glm::mat4& transformMatrix) {
        if (model->meshes.empty()) return btVector3(0.0f, 0.0f, 0.0f);

        glm::vec3 globalMin = glm::vec3(FLT_MAX);
        glm::vec3 globalMax = glm::vec3(-FLT_MAX);

        for (Mesh& mesh : model->meshes) {
            glm::vec3 min, max;
            mesh.GetBoundingBox(min, max, transformMatrix);
            globalMin = glm::min(globalMin, min);
            globalMax = glm::max(globalMax, max);
        }

        glm::vec3 center = (globalMin + globalMax) / 2.0f;
        return btVector3(center.x, center.y, center.z);
    }

};
