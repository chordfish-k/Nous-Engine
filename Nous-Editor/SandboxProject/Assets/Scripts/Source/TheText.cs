using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Nous;

namespace Sandbox
{
    internal class TheText : Entity
    {
        private CTextRenderer m_TextRenderer = null;

        public string Text
        {
            get
            {
                return m_TextRenderer != null ? m_TextRenderer.Text : "";
            }
            set
            {
                if (m_TextRenderer != null)
                    m_TextRenderer.Text = value;
            }
        }

        void OnCreate()
        {
            m_TextRenderer = GetComponent<CTextRenderer>();
        }
    }
}
