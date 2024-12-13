#include "IGraphicsEngine.h"
#include "GraphicsEngine.h"

void CreateGrapicsEngine(IGraphicsEngine** _output)
{
	(*_output) = new GraphicsEngine();
}