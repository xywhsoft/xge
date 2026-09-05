int xgeParticleDefinitionLoad(xge_particle_definition *out, const char *uri, int xson, char *error,
                              size_t capacity)
{
	xge_resource_t resource;
	int result;
	if (!out)
		return XGE_ERROR_INVALID_ARGUMENT;
	*out = NULL;
	if (error && capacity)
		error[0] = 0;
	memset(&resource, 0, sizeof(resource));
	result = xgeResourceLoad(uri, &resource);
	if (result != XGE_OK)
	{
		if (error && capacity)
			snprintf(error, capacity, "Unable to load particle resource");
		return result;
	}
	result = xgeParticleDefinitionParse(out, (const char *)resource.pData, (size_t)resource.iSize, xson,
	                                    error, capacity);
	xgeResourceFree(&resource);
	return result;
}
