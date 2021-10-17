#include "pch.h"
#include "Camera.h"
using namespace Elite;

Camera* Camera::m_pInstance{ nullptr };

Camera* Camera::GetInstance(uint32_t width, uint32_t height,Elite::FPoint3 pos,Elite::FVector3 dir)
{
	if (!m_pInstance)
	{
		m_pInstance = new Camera(width,height,pos,dir);
	}
	return m_pInstance;
}

Camera::Camera(uint32_t width, uint32_t height, Elite::FPoint3 pos, Elite::FVector3 dir)
	:m_WorldUpVector{ 0,1,0 }
	, m_Position{ pos.x,pos.y,-pos.z }
	, m_ForwardVector{ dir }
	, m_AngleOfFOV{ 60.f }
	, m_FOV{ tanf(Elite::ToRadians(m_AngleOfFOV / 2.f)) }

{
	m_UpVector={ 0.f, 1.f, 0.f }; 
	m_RightVector={ 1.f, 0.f, 0.f }; 
	m_ZFar = 100;
	m_ZNear = 0.1f;

	m_AspectRatio = float(width) / height;

}

float Camera::GetFOV() const
{
    return m_FOV;
}

Elite::FMatrix4 Camera::GetLeftHandedViewMatrix()
{
	// Left handed coordinate system
	//
    Normalize(m_ForwardVector);
    m_RightVector = Cross(m_WorldUpVector, m_ForwardVector);
    Normalize(m_RightVector);
    m_UpVector = Cross(m_ForwardVector, m_RightVector);
    Normalize(m_UpVector);

	return Inverse(FMatrix4{ m_RightVector.x, m_UpVector.x, m_ForwardVector.x,m_Position.x
							,m_RightVector.y, m_UpVector.y, m_ForwardVector.y,m_Position.y
							,m_RightVector.z, m_UpVector.z, m_ForwardVector.z,m_Position.z
							,0			  ,		0	  ,		0		   ,		1 });
}

Elite::FMatrix4 Camera::GetRightHandedViewMatrix()
{
	// Right handed view matrix
	Normalize(m_ForwardVector);
	m_RightVector = Cross(m_WorldUpVector, m_ForwardVector);
	Normalize(m_RightVector);
	m_UpVector = Cross(m_ForwardVector, m_RightVector);
	Normalize(m_UpVector);

	return { m_RightVector.x, m_UpVector.x, m_ForwardVector.x,m_Position.x
			 ,m_RightVector.y, m_UpVector.y, m_ForwardVector.y,m_Position.y
			 ,m_RightVector.z, m_UpVector.z, m_ForwardVector.z,m_Position.z
			 ,0			  ,		0	  ,		0		   ,		1 };
}

Elite::FMatrix4 Camera::GetLeftHandedProjectionMatrix()
{
	// Left handed projection matrix	
	//m_ZFar = 100.f;
	//m_ZNear = 0.1f;

	Elite::FMatrix4 projectionMatrix = FMatrix4::Identity();

	projectionMatrix.data[0][0] = 1.f / (m_AspectRatio * m_FOV);
	projectionMatrix.data[1][1] = 1.f / (m_FOV);
	projectionMatrix.data[2][2] = m_ZFar / (m_ZFar - m_ZNear);
	projectionMatrix.data[3][2] = -(m_ZFar * m_ZNear) / (m_ZFar - m_ZNear);
	projectionMatrix.data[2][3] = 1.f;
	projectionMatrix.data[3][3] = 0;

	return projectionMatrix;
}
Elite::FMatrix4 Camera::GetRightHandedProjectionMatrix()
{
	// Left handed projection matrix
	//m_ZFar = 0.1f;
	//m_ZNear = 100.f;
	Elite::FMatrix4 projectionMatrix = FMatrix4::Identity();

	projectionMatrix.data[0][0] = 1.f / (m_AspectRatio * m_FOV);
	projectionMatrix.data[1][1] = 1.f / (m_FOV);
	projectionMatrix.data[2][2] = m_ZFar / (m_ZNear - m_ZFar);
	projectionMatrix.data[3][2] = (m_ZFar * m_ZNear) / (m_ZNear - m_ZFar);	
	projectionMatrix.data[2][3] = -1.f;
	projectionMatrix.data[3][3] = 0;

	return projectionMatrix;
}

void Camera::TranslateX(float distanceX)
{
	m_Position += m_RightVector * distanceX;
}
void Camera::TranslateY(float distanceY)
{
	m_Position += m_UpVector * distanceY;
}
void Camera::TranslateZ(float distanceZ)
{
	m_Position += m_ForwardVector * distanceZ;
}
float Camera::GetNearPlane()
{
	return m_ZNear;
}
float Camera::GetFarPlane()
{
	return m_ZFar;
}
void Camera::Pitch(float angle)
{
	//x
    m_ForwardVector = Inverse(Transpose(MakeRotation(Elite::ToRadians(angle), m_RightVector))) * m_ForwardVector;
}

void Camera::Yaw(float angle)
{
	//y
	m_ForwardVector = Inverse(Transpose(MakeRotation(Elite::ToRadians(angle), m_UpVector))) * m_ForwardVector;
}
void Camera::Roll(float angle)
{
	//z
	m_ForwardVector = Inverse(Transpose(MakeRotation(Elite::ToRadians(angle), -m_ForwardVector))) * m_ForwardVector;
}

Elite::FPoint4 Camera::GetPosition() const
{
    return m_Position;
}

void Camera::SetPosition(const Elite::FPoint4& p)
{
	m_Position = p;
}

Elite::FVector3 Camera::GetForwardVector() const
{
	return m_ForwardVector;
}
void Camera::SetForwardVector(const Elite::FVector3& v)
{
	m_ForwardVector = v;
}

float Camera::GetAspectRatio() const
{
	return m_AspectRatio;
}
