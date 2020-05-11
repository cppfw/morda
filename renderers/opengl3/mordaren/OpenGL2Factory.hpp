#pragma once

#include <morda/render/Factory.hpp>

#include "OpenGL2ShaderPosTex.hpp"


class OpenGL2Factory : public morda::Factory{
public:
	OpenGL2Factory();
	
	OpenGL2Factory(const OpenGL2Factory&) = delete;
	OpenGL2Factory& operator=(const OpenGL2Factory&) = delete;
	
	virtual ~OpenGL2Factory()noexcept;

	std::shared_ptr<morda::Texture2D> createTexture2D(morda::Texture2D::TexType_e type, kolme::Vec2ui dim, utki::span<const std::uint8_t>& data) override;

	std::shared_ptr<morda::VertexBuffer> createVertexBuffer(utki::span<const kolme::Vec4f> vertices) override;
	
	std::shared_ptr<morda::VertexBuffer> createVertexBuffer(utki::span<const kolme::Vec3f> vertices) override;
	
	std::shared_ptr<morda::VertexBuffer> createVertexBuffer(utki::span<const kolme::Vec2f> vertices) override;

	std::shared_ptr<morda::IndexBuffer> createIndexBuffer(utki::span<const std::uint16_t> indices) override;
	
	std::shared_ptr<morda::VertexArray> createVertexArray(std::vector<std::shared_ptr<morda::VertexBuffer>>&& buffers, std::shared_ptr<morda::IndexBuffer> indices, morda::VertexArray::Mode_e mode) override;

	std::unique_ptr<Shaders> createShaders() override;
	
	std::shared_ptr<morda::FrameBuffer> createFramebuffer(std::shared_ptr<morda::Texture2D> color) override;
};
