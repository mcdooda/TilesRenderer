#pragma once

#include <GL/glew.h>

struct DrawElementsIndirectCommand
{
	GLuint count;
	GLuint instanceCount;
	GLuint firstIndex;
	GLuint baseVertex;
	GLuint baseInstance;
};

class GLBufferBase
{
public:
	GLBufferBase()
	{
		glCreateBuffers(1, &m_handle);
	}

	~GLBufferBase()
	{
		glDeleteBuffers(1, &m_handle);
	}

	void bind(GLenum target, GLuint index)
	{
		glBindBufferBase(target, index, m_handle);
	}

	void bindRange(GLenum target, GLuint index, GLintptr offset, GLsizeiptr size)
	{
		glBindBufferRange(target, index, m_handle, offset, size);
	}

	GLuint getHandle() const { return m_handle; }

protected:
	GLuint m_handle;
};

class GLBuffer : public GLBufferBase
{
public:
	GLBuffer() = default;

	GLBuffer(const void* data, GLsizeiptr size)
	{
		update(data, size);
	}

	void update(const void* data, GLsizeiptr size)
	{
		glNamedBufferStorage(m_handle, size, data, 0);
	}
};

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
class GLTypedBufferBase : public GLBufferBase
{
public:
	constexpr GLsizeiptr getSize() const { return sizeof(T); }
};

template <class T>
class GLImmutableBuffer : public GLTypedBufferBase<T>
{
public:
	GLImmutableBuffer(const T& data)
	{
		glNamedBufferStorage(m_handle, getSize(), &data, 0);
	}
};

template <class T>
class GLMutableBuffer : public GLTypedBufferBase<T>
{
public:
	GLMutableBuffer(const T* buffer = nullptr)
	{
		glNamedBufferStorage(m_handle, getSize(), buffer, GL_DYNAMIC_STORAGE_BIT);
	}

	void update(const T& object)
	{
		update(0, &object, getSize());
	}

	void update(GLintptr offset, const void* data, GLsizeiptr size)
	{
		assert(0 <= offset && offset < getSize());
		assert(offset + size <= getSize());
		glNamedBufferSubData(
			m_handle,
			offset,
			size,
			data
		);
	}
};

template <class T, int MaxElements>
class GLArrayBuffer : public GLMutableBuffer<T[MaxElements]>
{
public:
	GLArrayBuffer()
	{
		m_objects.reserve(getSize());
	}

	void clearObjects()
	{
		m_objects.clear();
	}

	void addObject(const T& object)
	{
		m_objects.emplace_back(object);
		assert(static_cast<GLsizeiptr>(m_objects.size() * sizeof(T)) <= getSize());
	}

	T& addObject()
	{
		T& object = m_objects.emplace_back();
		assert(static_cast<GLsizeiptr>(m_objects.size() * sizeof(T)) <= getSize());
		return object;
	}

	void upload()
	{
		update(
			0,
			m_objects.data(),
			m_objects.size() * sizeof(T)
		);
	}

	size_t getObjectCount() const { return m_objects.size(); }

private:
	std::vector<T> m_objects;
};

template <int MaxElements>
class GLIndirectCommandsBuffer : public GLArrayBuffer<DrawElementsIndirectCommand, MaxElements>
{
public:
	void clearCommands()
	{
		clearObjects();
	}

	void addCommand(
		GLuint count,
		GLuint instanceCount,
		GLuint firstIndex,
		GLuint baseVertex,
		GLuint baseInstance
	)
	{
		DrawElementsIndirectCommand& command = addObject();
		command.count = count;
		command.instanceCount = instanceCount;
		command.firstIndex = firstIndex;
		command.baseVertex = baseVertex;
		command.baseInstance = baseInstance;
	}

	void draw()
	{
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_handle);
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(getObjectCount()), 0);
	}
};
