/*
 * Created on Mar 21, 2004
 *
 */
package org.mod_pubsub.client;

/**
 * @author rtl
 *
 */
public interface Constants {

	public static final String KN_TO_PARAM = "kn_to";
	public static final String KN_FROM_PARAM = "kn_from";
	public static final String KN_ID_PARAM = "kn_id";
	public static final String KN_METHOD_PARAM = "do_method";

	public static final String KN_SIMPLE_RESPONSE_FORMAT = "simple";
	public static final String KN_RESPONSE_FORMAT_PARAM = "kn_response_format";
	public static final String KN_STATUS_FROM_PARAM = "kn_status_from";
	public static final String KN_TO_HTTP_STREAM = "javascript:";

	public static final String KN_PAYLOAD_PARAM = "kn_payload";

	public static final String KN_ROUTE_URI = "kn_uri";
	public static final String KN_ROUTE_LOCATION = "kn_route_location";
	public static final String KN_ROUTES_PATH = "/kn_routes/";
	
	public static final String KN_EXPIRES_PARAM = "kn_expires";

	public static final String KN_MAX_AGE_PARAM = "do_max_age";
	public static final String KN_MAX_COUNT_PARAM = "do_max_n";
	public static final String KN_SINCE_CHECKPOINT_PARAM = "do_since_checkpoint";

	public static final String DEFAULT_USERNAME = "anonymous";

	public static final String USER_AGENT = "java.mod_pubsub.client/0.1";
	public static final String KN_COMMAND_CONTENT_TYPE =
		"application/x-www-form-urlencoded; charset=UTF-8";

}
