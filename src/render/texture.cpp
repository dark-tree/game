
#include "texture.hpp"

/*
 * Texture
 */

GLenum Texture::format(int channels) {
	switch (channels) {
		case 4: return GL_RGBA;
		case 3: return GL_RGB;
		case 1: return GL_ALPHA;
	}

	fault("Unsuported texture channel count: %d!\n", channels);
}

void Texture::framebuffer(GLenum attachment) const {
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, tid, 0);
}

Texture::Texture(const char* path)
: Texture() {
	int32_t width, height, channels;
	uint8_t* data = stbi_load(path, &width, &height, &channels, 4);

	if (data == nullptr) {
		fault("Failed to load texture: '%s'!\n", path);
	}

	upload(data, width, height, 4);
	stbi_image_free(data);
}

Texture::Texture() {
	glGenTextures(1, &tid);
	use();

	// set texture wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

Texture::~Texture() {
	glDeleteTextures(1, &tid);
}

void Texture::upload(unsigned char* data, int width, int height, int channels) {
	use();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format(channels), GL_UNSIGNED_BYTE, data);
	w = width;
	h = height;
}

void Texture::resize(int width, int height, GLenum internal_format, GLenum format) {
	use();
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
	w = width;
	h = height;
}

void Texture::use() const {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tid);
}

uint32_t Texture::width() const {
	return w;
}

uint32_t Texture::height() const {
	return h;
}

/*
 * RenderBuffer
 */

void RenderBuffer::framebuffer(GLenum attachment) const {
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rbo);
}

RenderBuffer::RenderBuffer() {
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
}

RenderBuffer::~RenderBuffer() {
	glDeleteRenderbuffers(1, &rbo);
}

void RenderBuffer::resize(int width, int height, GLenum internal_format, GLenum format) {
	use();
	glRenderbufferStorage(GL_RENDERBUFFER, internal_format, width, height);
	w = width;
	h = height;
}

void RenderBuffer::use() const {
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
}

uint32_t RenderBuffer::width() const {
	return w;
}

uint32_t RenderBuffer::height() const {
	return h;
}

/*
 * TileSet
 */

TileSet::Ref TileSet::of(int x, int y) {
	return {x, y};
}

void TileSet::framebuffer(GLenum attachment) const {
	texture.framebuffer(attachment);
}

TileSet::TileSet(const char* path, uint32_t tile)
: TileSet(path, tile, tile) {}

TileSet::TileSet(const char* path, uint32_t tw, uint32_t th)
: texture(path), tw(tw), th(th), line(texture.width() / tw), column(texture.height() / th) {
	if (texture.width() % tw != 0 || texture.height() % th != 0) {
		fault("Unable to neatly divide tileset! Texture width: %d tile width: %d\n", texture.width(), tw);
	}
}

void TileSet::resize(int w, int h, GLenum internal_format, GLenum format) {
	fault("Can't resize tileset, operation forbidden!");
}

void TileSet::use() const {
	texture.use();
}

uint32_t TileSet::width() const {
	return texture.width();
}

uint32_t TileSet::height() const {
	return texture.height();
}

uint32_t TileSet::columns() const {
	return line;
}

uint32_t TileSet::rows() const {
	return column;
}

Sprite TileSet::sprite(int x, int y) {
	const int min_x = x * tw;
	const int min_y = y * th;
	const int max_x = min_x + tw;
	const int max_y = min_y + th;

	return {
		(min_x + 0.01f) / (float) width(),
		(min_y + 0.01f) / (float) height(),
		(max_x - 0.01f) / (float) width(),
		(max_y - 0.01f) / (float) height()
	};
}

Sprite TileSet::sprite(int index) {
	int x = index % line;
	int y = column - (index / line) - 1;

	return sprite(x, y);
}

Sprite TileSet::sprite(Ref ref) {
	return sprite(ref.x, ref.y);
}
