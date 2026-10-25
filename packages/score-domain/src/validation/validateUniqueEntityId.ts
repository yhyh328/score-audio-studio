import type { EntityId } from "../model/entityId.js";
import type { ValidationIssue } from "./validationTypes.js";

export function isUniqueEntityId(
  id: EntityId, usedEntityIds: Set<EntityId>): boolean {
  if (usedEntityIds.has(id)) return false;
  usedEntityIds.add(id);
  return true;
}

export function entityIdIssue(
  id: EntityId, path: string): ValidationIssue {
  return {
    target: "entityId",
    severity: "error",
    code: "DUPLICATE_ENTITY_ID",
    message: `Entity ID must be unique: ${id}`,
    path: `${path}.id`,
  }
}
