/**
 * Unique identifier for domain entities.
 * Format: `${entityType}-${randomUUID}`
 *
 * Examples:
 * - `part-550e8400-e29b-41d4-a716-446655440000`
 * - `measure-550e8400-e29b-41d4-a716-446655440000`
 * - `event-550e8400-e29b-41d4-a716-446655440000`
 */
export type EntityId = string;
