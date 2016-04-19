<?php

/**
 * @file
 * Contains IntegrationTestAbstractMigration class.
 */

use Drupal\integration_migrate\MigrateItemJSON;
use Drupal\integration_migrate\AbstractMigration;
use Drupal\integration_migrate\DocumentWrapperInterface;


/**
 * Class IntegrationTestAbstractMigration.
 */
abstract class EuropaEuAbstractMigration extends AbstractMigration {

  /**
   * Define source key, to be used in setMap().
   *
   * @return array
   *    Get default source key definition.
   */
  public function getSourceKey() {
    return array(
      '_id' => array(
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
    );
  }

  /**
   * Constructor.
   *
   * @param string $type
   *    Document type, used to identify the correct fixture directory.
   * @param mixed $arguments
   *    Default constructor arguments.
   */
  public function __construct($type, $arguments) {
    parent::__construct($arguments);

    $base_path = self::getFixturesPath() . '/' . $type;
    $this->setSource(new MigrateSourceList(
      new \MigrateListJSON($base_path . '/list.json'),
      new MigrateItemJSON($base_path . '/document-:id.json', array()),
      array()
    ));

    $this->addFieldMapping('metatag_description', 'meta_description');
    $this->addFieldMapping('metatag_keywords', 'meta_keywords');
    $this->addFieldMapping('metatag_title', 'meta_title');

  }

  /**
   * Return Consumer tests fixtures path.
   *
   * @return string
   *    Fixtures path.
   */
  public static function getFixturesPath() {
    return drupal_get_path('module', 'europaeu_migrate') . '/fixtures';
  }

  /**
   * Implements Migration::complete() callback.
   *
   * @param object $entity
   *    Entity object.
   * @param \stdClass $source_row
   *    Source row, as expected by Migrate class.
   */
  public function complete($entity, \stdClass $source_row) {
    // Set the publication through workbench_moderation when enabled.
    if (isset($entity->workbench_moderation)) {
      workbench_moderation_moderate($entity, 'published');
    }
    if (entity_translation_enabled($this->getDestination()->getEntityType())) {
      $this->saveTranslations($entity, $source_row);
    }
    foreach ($source_row->getAvailableLanguages() as $language) {
      if ($language != $source_row->getDefaultLanguage()) {
        $entity_type = $this->getDestination()->getEntityType();
        // Generate aliases for the translations.
        if (module_exists('pathauto') && is_callable('pathauto_' . $entity_type . '_update_alias')) {
          call_user_func('pathauto_' . $entity_type . '_update_alias', $entity, 'update', array('language' => $language));
        }
      }
    }
  }


  /**
   * Save field translations for the specified entity.
   *
   * @param object $entity
   *    Entity object.
   * @param DocumentWrapperInterface $source_row
   *    Source row, as expected by Migrate class.
   */
  private function saveTranslations($entity, DocumentWrapperInterface $source_row) {

    foreach ($source_row->getAvailableLanguages() as $language) {

      if ($language != $source_row->getDefaultLanguage()) {

        $source_row->setSourceValues($language);
        $this->sourceValues = $source_row;
        $this->prepareRow($this->sourceValues);
        $this->applyMappings();

        // Prepare entity in order to correctly apply mappings in prepare().
        $new_entity = clone $entity;
        foreach ((array) $this->destinationValues as $field_name => $value) {
          $new_entity->$field_name = $value;
        }
        $this->getDestination()->prepare($new_entity, $this->sourceValues);

        $entity_type = $this->getDestination()->getEntityType();
        $bundle_name = $this->getDestination()->getBundle();

        $values = [];
        $field_instances = field_info_instances($entity_type, $bundle_name);
        foreach ($field_instances as $field_name => $field_instance) {
          if (isset($this->destinationValues->$field_name)) {
            $values[$field_name] = $new_entity->$field_name;
          }
        }

        // Apply translations, moving this into a separate method.
        $translation_handler = entity_translation_get_handler($entity_type, $entity);

        // Load translations.
        $translation_handler->loadTranslations();

        $translation = [
          'translate' => 0,
          'status' => TRUE,
          'language' => $language,
          'source' => $entity->translations->original,
          'changed' => time(),
        ];

        // Add the new translation and store it.
        $translation_handler->setTranslation($translation, $values);

        // Preserve original language setting.
        $entity->field_language = $entity->language;
        $entity->language = $entity->translations->original;

        // Save entity.
        $entity->status = NODE_PUBLISHED;
        unset($entity->path['alias']);
        entity_save($entity_type, $entity);
      }
    }
  }

}
