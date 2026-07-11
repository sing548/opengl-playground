#include "con-trail.h"

#include <vector>
#include <cstddef>
#include <filesystem>

#include "../../../engine/shaders/shader.h"
#include "../../../engine/helpers/file-helper.h"
#include "../../../engine/rendering/render-list.h"

ConTrail::ConTrail(GameWorld& world) : world_(world)
{
    std::string vert = (std::filesystem::path(FileHelper::GetShaderDir()) / "con-trail.vert").string();
    std::string frag = (std::filesystem::path(FileHelper::GetShaderDir()) / "con-trail.frag").string();
    shader_ = std::make_unique<Shader>(vert.c_str(), frag.c_str());

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &pointsVBO_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, pointsVBO_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(ConVertData), (void*)offsetof(ConVertData, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ConVertData), (void*)offsetof(ConVertData, angle));

    glBindVertexArray(0);

    wingtipOffsets_ =
    {
        glm::vec3(0, 0.4f, 0.4f),
        glm::vec3(0, 0.4f, -0.4f),
        glm::vec3(0, -0.4f, -0.4f),
        glm::vec3(0, -0.4f, 0.4f)
    };
}

ConTrail::~ConTrail()
{
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &pointsVBO_);
}

void ConTrail::Render(const FrameGlobals& globals)
{
    float width = 0.04f;

    shader_->Use();

    shader_->SetVec3("viewPos", globals.cameraPos);
	shader_->SetMat4("projection", globals.projection);
	shader_->SetMat4("view", globals.view);

    shader_->SetFloat("uMaxAge", MAX_AGE);

    BuildTrails();

    std::vector<ConVertData> points;
    std::vector<std::pair<GLint, GLsizei>> ranges;

    std::vector<float> times;
    std::vector<float> speeds;
    std::vector<float> angles;

    auto now = std::chrono::steady_clock::now();

    for (auto& [id, queue] : trails_)
    {
        if (queue.size() < 2) continue;

        for (int i = 0; i < 4; ++i)
        {
            ranges.push_back({ (GLint)points.size(), (GLsizei)(queue.size() - 1) * 2 });

            for (int j = 1; j < queue.size(); ++j)
            {
                auto segmentDir = glm::normalize(queue[j].Positions[i] - queue[j-1].Positions[i]);
                auto toCam = globals.cameraPos - queue[j].Positions[i];
                auto perp = glm::cross(segmentDir, toCam);

                float len = glm::length(perp);

                auto perpDir = (len > 1e-5f)
                    ? perp/len
                    : glm::normalize(glm::cross(segmentDir, glm::vec3(0,1,0)));

                float age = std::chrono::duration<float>(now - queue[j].CreationTime).count();
                float w = width * (1.0f + age * 3.0f);
                points.push_back({ queue[j].Positions[i] - perpDir * w, age, queue[j].Angle, queue[j].Velocity, +1.0f });
                points.push_back({ queue[j].Positions[i] + perpDir * w, age, queue[j].Angle, queue[j].Velocity, -1.0f });
            }
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, pointsVBO_);
    glBufferData(GL_ARRAY_BUFFER, points.size()*sizeof(ConVertData), points.data(), GL_DYNAMIC_DRAW);
    glBindVertexArray(vao_);

    for (auto& [offset, count] : ranges)
        glDrawArrays(GL_TRIANGLE_STRIP, offset, count);
    
    glBindVertexArray(0);
}

void ConTrail::BuildTrails()
{
    std::vector<uint32_t> updatedIds;
    
    for (auto& [id, _] : world_.GetPlayerData())
    {
        ExtendDeque(id);
        updatedIds.push_back(id);
    }

    for (auto& [id, _] : world_.GetNpcData())
    {
        ExtendDeque(id);
        updatedIds.push_back(id);
    }

    if (updatedIds.size() != trails_.size())
        // ToDo: Add check if updatedIds or trails_ grow in size to instead use unordered_set for performance
        for (auto it = trails_.begin(); it != trails_.end();)
            if (!std::binary_search(updatedIds.begin(), updatedIds.end(), it->first))
                it = trails_.erase(it);
            else
                ++it;
}

void ConTrail::ExtendDeque(uint32_t id)
{
    auto now = std::chrono::steady_clock::now();
    auto& model = world_.GetScene().GetModelByReference(id);
    const auto& interpolatedPi = model.GetInterpolatedInfo();

    glm::vec3 pos = interpolatedPi.position_;
    glm::quat rot = interpolatedPi.rotation_;

    auto& queue = trails_[id];

    QueueData data {
        {
            pos + rot * wingtipOffsets_[0],
            pos + rot * wingtipOffsets_[1],
            pos + rot * wingtipOffsets_[2],
            pos + rot * wingtipOffsets_[3],
        },
        glm::length(model.GetRotationSpeed()),
        glm::length(model.GetVelocity()),
        now
    };

    while (!queue.empty() && std::chrono::duration<float>(now - queue.front().CreationTime).count() > MAX_AGE)
        queue.pop_front();

    if (queue.size() == 0)
        queue.push_back(data);
    else if (glm::length(data.Positions[0] - queue.back().Positions[0]) > (float)TRAIL_SEGMENT_LENGTH / 4)
        queue.push_back(data);
}
