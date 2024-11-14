namespace Nous
{
    public struct Prefab// : Asset
    {
        public ulong Handle;

        public Prefab(ulong handle)
        {
            Handle = handle;
        }

        public bool IsValid()
        {
            return Handle != 0;
        }

        public string GetFilePath()
        {
            if (IsValid())
            {
                return InternalCalls.Prefab_GetFilePath(Handle);
            }
            return "";
        }

        public Entity Instantate()
        {
            InternalCalls.Prefab_Instantate(Handle, out ulong entityID);
            return new Entity(entityID);
        }
    }
}
