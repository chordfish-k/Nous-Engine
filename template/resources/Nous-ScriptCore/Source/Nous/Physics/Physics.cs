using System;

namespace Nous
{
    public static class Physics
    {
        public static void DisableLastContact()
        {
            InternalCalls.Physics_DisableLastContact();
        }
    }

    public struct CollisionContact
    {
        private ulong m_Ptr;

        public CollisionContact(ulong contactPtr)
        {
            m_Ptr = contactPtr;
        }

        public bool Enable
        {
            get
            {
                return InternalCalls.Physics_Contact_IsEnable(m_Ptr);
            }
            set 
            {
                InternalCalls.Physics_Contact_SetEnable(m_Ptr, value);
            }
        }
    }
}
