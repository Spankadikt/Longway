﻿using System;
using System.Windows.Forms;
using Tao.OpenGl;
using Tao.Platform.Windows;
using Fawrek.Wrapper;

namespace Fawrek.Editor
{
    class CustomOpenGLControl : SimpleOpenGlControl
    {
        private double m_dScaleX = 1.0;
        private double m_dScaleY = 1.0;

        IntPtr fawrekPtr;

        public CustomOpenGLControl()
        {
            InitializeContexts();

            fawrekPtr = Fawrek.Wrapper.Wrapper.RunFawrekCreate();
            int initResult = Fawrek.Wrapper.Wrapper.RunFawrekInit(fawrekPtr);

        }

        protected override void OnPaint(PaintEventArgs e)
        {
            Fawrek.Wrapper.Wrapper.RunFawrekRender(fawrekPtr);

            SwapBuffers();

            //call this to refresh and draw
            Draw();
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            m_dScaleX = LogScaleX / 96.0;
            m_dScaleY = LogScaleY / 96.0;
            mIsInitialized = true;
        }

        private bool mIsInitialized = false;

        protected override void OnResize(EventArgs e)
        {
            if (mIsInitialized)
            {
                MakeCurrent();
                int iWidth = (int)m_dScaleX * Width;
                int iHeight = (int)m_dScaleY * Height;

                Gl.glViewport(0, 0, iWidth, iHeight);

                Gl.glMatrixMode(Gl.GL_PROJECTION);
                Gl.glLoadIdentity();
                Gl.glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

                Gl.glMatrixMode(Gl.GL_MODELVIEW);
                Gl.glLoadIdentity();
            }
        }

        protected override void OnPaintBackground(PaintEventArgs e)
        {
            return;
        }
    }
}
