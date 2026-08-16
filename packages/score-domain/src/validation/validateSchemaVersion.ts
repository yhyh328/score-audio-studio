/**
 * Schema version history:
 *
 * - ProjectDocument
 *   - Version 1: Initial version (in development; finalization date TBD)
 *
 * - ScoreDocument
 *   - Version 1: Initial version (in development; finalization date TBD)
 */

const MAX_SCHEMA_VERSIONS = {
    score  : 1,
    project: 1
}

type SchemaTarget = keyof typeof MAX_SCHEMA_VERSIONS;

export function isValidSchemaVersion(target: SchemaTarget, schemaVersion: number): boolean {
    const maxVersion  = MAX_SCHEMA_VERSIONS[target];
    return (maxVersion !== undefined        // check if the target is valid   
        &&  Number.isInteger(schemaVersion) // check if the schemaVersion is an integer
        &&  1 <= schemaVersion              // check if the schemaVersion is positive
        &&  schemaVersion <= maxVersion     // check if the schemaVersion is not greater than the max version
    );
}