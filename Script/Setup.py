# from https://github.com/TheCherno/Hazel
import os
import subprocess
import platform

from SetupPython import PythonConfiguration as PythonRequirements

# Make sure everything we need for the setup is installed
PythonRequirements.Validate()

from SetupVulkan import VulkanConfiguration as VulkanRequirements

os.chdir('./../') # Change from devtools/scripts directory to root

VulkanRequirements.Validate()
