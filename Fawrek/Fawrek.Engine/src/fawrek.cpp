#include <fawrek.h>

Fawrek::Fawrek()
{
	m_mouseLeftButton._bClicked = false;
}

Fawrek::~Fawrek()
{
    Dispose();
}

int Fawrek::Init()
{
	GLint GlewInitResult = glewInit();
	if (GLEW_OK != GlewInitResult) 
	{
		printf("ERROR: %s\n",glewGetErrorString(GlewInitResult));
		return 100+GlewInitResult;
	}

	int initGlResult = InitGL();
	if(initGlResult != 1)
		return initGlResult;

	pDirectionalLight = new DirectionalLight();

    pDirectionalLight->m_color = Vector3(1.0f, 1.0f, 1.0f);
    pDirectionalLight->m_fAmbientIntensity = 0.85f;
    pDirectionalLight->m_fDiffuseIntensity = 0.2f;
    pDirectionalLight->m_direction = Vector3(0.0f, 1.0f, 1.0f);     

	pCamera = new Camera();

    pCamera->PerspectiveFOV(120.0f,4/3,0.01f,100.0f);
	pCamera->LookAt(pCamera->pos,pCamera->target,pCamera->up);

	pPickingTexture = new PickingTexture();

	if(!pPickingTexture->Init(640,480))
		return 30;

	//pSkinningRoutine = new SkinningRoutine("shaders/skinningroutine.vs","shaders/skinningroutine.fs");
	pLightingRoutine = new LightingRoutine("shaders/lightingroutine.vs","shaders/lightingroutine.fs");
	pColoringRoutine = new ColoringRoutine("shaders/coloringroutine.vs","shaders/coloringroutine.fs");
	pPickingRoutine = new PickingRoutine("shaders/pickingroutine.vs","shaders/pickingroutine.fs");

	//int lightInit = pSkinningRoutine->Init();
	int lightInit = pLightingRoutine->Init();
	int colorInit = pColoringRoutine->Init();
	int pickInit = pPickingRoutine->Init();

	if (lightInit != 0)
		return lightInit;

	if (pickInit != 0)
		return pickInit;

	if (colorInit != 0)
		return pickInit;


	pColoringRoutine->Enable();

	pLightingRoutine->Enable();
	pLightingRoutine->SetTextureUnit(0);
	pLightingRoutine->SetDirectionalLight(pDirectionalLight);
	pLightingRoutine->SetMatSpecularIntensity(1.0f);
	pLightingRoutine->SetMatSpecularPower(32);

	//pSkinningRoutine->Enable();
	//pSkinningRoutine->SetTextureUnit(0);
	//pSkinningRoutine->SetDirectionalLight(pDirectionalLight);
	//pSkinningRoutine->SetMatSpecularIntensity(1.0f);
	//pSkinningRoutine->SetMatSpecularPower(32);

	pScene = new Scene();
	pScene->Load("resources/scene_demo.xml");

	/*for(int i = 0 ; i < pScene->m_pObjectManager->m_objects.size() ; i++ )
	{
		Model* pModel = static_cast<Model*>(pScene->m_pObjectManager->m_objects[i]);
		pModel->m_pAnimation->CrossfadeToClip(18);
		pModel->m_pAnimationBis->CrossfadeToClip(18);
	}*/


	return 0;
}

void Fawrek::Dispose()
{
	SAFE_DELETE(pDirectionalLight);
	SAFE_DELETE(pSkinningRoutine);
    SAFE_DELETE(pLightingRoutine);
	SAFE_DELETE(pColoringRoutine);
	SAFE_DELETE(pPickingRoutine);
    SAFE_DELETE(pCamera);
	SAFE_DELETE(pScene);
}

void Fawrek::Render()
{
	float runningTime = GetRunningTime();

	//picking
	pPickingTexture->EnableWriting();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	pPickingRoutine->Enable();

    for(int i = 0 ; i < pScene->m_pObjectManager->m_objects.size() ; i++ )
	{
        pPickingRoutine->SetObjectIndex(i);
        Model* pModel = static_cast<Model*>(pScene->m_pObjectManager->m_objects[i]);
		pModel->Render(pCamera,pPickingRoutine,runningTime);
    }

    pPickingTexture->DisableWriting(); 


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//coloring?
	if(m_mouseLeftButton._bClicked)
	{
		Pixel = pPickingTexture->ReadPixel(m_mouseLeftButton._nX, 480 - m_mouseLeftButton._nY - 1);

		for(int i = 0 ; i < pScene->m_pObjectManager->m_objects.size() ; i++ )
		{
			Model* pModel = static_cast<Model*>(pScene->m_pObjectManager->m_objects[i]);
			pModel->m_bSelected = false;
		}

        if (Pixel.PrimID != 0)
		{
			Model* pModel = static_cast<Model*>(pScene->m_pObjectManager->m_objects[Pixel.ObjectID]);
			pModel->m_bSelected = true;
        }

		m_mouseLeftButton._bClicked = false;
	}

	//rendering
	for(int i = 0 ; i < pScene->m_pObjectManager->m_objects.size() ; i++ )
	{
		Model* pModel = static_cast<Model*>(pScene->m_pObjectManager->m_objects[i]);
		if(true)
		{
			if(pModel->m_bSelected)
			{
				pColoringRoutine->Enable();
				pModel->Render(pCamera,pColoringRoutine,runningTime);
			}
			else
			{
				pLightingRoutine->Enable();
				pModel->Render(pCamera,pLightingRoutine,runningTime);
			}
		}
		else
		{
			if(pModel->m_bSelected)
			{	
				for(int i=0;i<pModel->m_pMesh->m_entries.size();i++)
				{
					Matrix modelMatrix = Matrix::Identity;

					modelMatrix.Translate(pModel->m_translation);
					Quaternion qRotate = qRotate.FromEuler(pModel->m_rotation.m_fX,pModel->m_rotation.m_fY,pModel->m_rotation.m_fZ);
					modelMatrix.Rotate(qRotate);
					modelMatrix.Scale(pModel->m_scale);

					Matrix modelView = pCamera->view * modelMatrix;
					Matrix viewProjection = pCamera->projection * modelView;

					if(i==(int)Pixel.DrawID)
					{
						pColoringRoutine->Enable();

						pColoringRoutine->SetWVP(viewProjection);
						pColoringRoutine->SetWorldMatrix(modelMatrix);


						pModel->m_pMesh->PickingRender((int)Pixel.ObjectID,i,(int)Pixel.PrimID);
					}
					else
					{
						pLightingRoutine->Enable();

						pLightingRoutine->SetWVP(viewProjection);
						pLightingRoutine->SetWorldMatrix(modelMatrix);

						pModel->m_pMesh->PickingRender((int)Pixel.ObjectID,i,(int)Pixel.PrimID);
					}
				}
			}
			else
			{
				pLightingRoutine->Enable();
				pModel->Render(pCamera,pLightingRoutine,runningTime);
			}
		}
	}
}

int Fawrek::InitGL()
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	return TRUE;										// Initialization Went OK
}

void Fawrek::KeyboardManager(bool _keys[256])
{	
	if (_keys[0x41])//A
	{
		//pModel->m_pAnimation->CrossfadeToClip(0);
        //pModel->m_pAnimationBis->CrossfadeToClip(0);
	}
	if (_keys[0x42])//Z
	{
		//pModel->m_pAnimation->CrossfadeToClip(18);
        //pModel->m_pAnimationBis->CrossfadeToClip(18);
	}
	if (_keys[0x41])//B
	{
        //pModel->m_pAnimationBis->CrossfadeToClip(2);
        //pModel->m_pAnimationBis->GetLastClip().Init();
        //pModel->m_pAnimationBis->QueueNextClip(&pModel->m_pAnimationBis->GetLastClip());
	}
    if (_keys[0x4E])
	{
		//pModel->m_pAnimation->CrossfadeToClip(2);
        //pModel->m_pAnimation->QueueNextClip(&pModel->m_pAnimation->GetLastClip());
        //pModel->m_pAnimationBis->CrossfadeToClip(2);
        //pModel->m_pAnimationBis->QueueNextClip(&pModel->m_pAnimationBis->GetLastClip());
	}
}

void Fawrek::MouseManager(int _nX,int _nY)
{
	m_mouseLeftButton._bClicked = true;
	m_mouseLeftButton._nX = _nX;
	m_mouseLeftButton._nY = _nY;
}

