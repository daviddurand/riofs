/*
 * ec2_metadata.h
 *
 *  Created on: Oct 5, 2016
 *      Author: carpenlc
 */

#ifndef INCLUDE_EC2_METADATA_H_
#define INCLUDE_EC2_METADATA_H_

/*
 * Memory structure used to hold data returned by the curl GET requests.
 */
typedef struct  {
	char *memory;
	size_t size;
} memory_structure;

typedef struct {
	char *last_updated;
	char *aws_access_key;
	char *aws_secret_access_key;
	char *aws_session_token;
	char *expiration;
} aws_credentials;

/*
 * Base URL for the EC2 instance metadata.
 */
const char *EC2_INSTANCE_META_DATA_URL = "http://instance-data/latest/meta-data/";

/*
 * Offset URL context for retrieving the temporary IAM role credentials.
 */
const char *EC2_IAM_ROLE_OFFSET = "iam/security-credentials/";

/*
 * JSON element containing the time when the IAM role credentials were last
 * updated.
 */
const char *JSON_AWS_CREDENTIAL_CREATED = "LastUpdated";

/*
 * JSON element containing the time when the IAM role credentials are set to
 * expire.
 */
const char *JSON_AWS_CREDENTIAL_EXPIRATION = "Expiration";

/*
 * JSON element containing the AWS access key.
 */
const char *JSON_AWS_ACCESS_KEY = "AccessKeyId";

/*
 * JSON element containing the AWS secret access key.
 */
const char *JSON_AWS_SECRET_ACCESS_KEY = "SecretAccessKey";

/*
 * JSON element containing the token
 */
const char *JSON_AWS_TOKEN = "Token";

/*
 * The number of tokens (i.e. tokens in the context of the jsmn JSON
 * processor) that are in the EC2 metadata response.  (Note: there are
 * actually only 14 tokens, however, the squirrelly jsmn JSON processor
 * requires one extra to handle the trailing bracket.)
 */
const int JSON_AWS_TOKEN_COUNT = 15;

/*
 * Prototype for function that will retrieve the AWS credential data
 * associated with the IAM role.
 */
void get_aws_credentials(aws_credentials *creds, char *iam_role);

#endif /* INCLUDE_EC2_METADATA_H_ */
