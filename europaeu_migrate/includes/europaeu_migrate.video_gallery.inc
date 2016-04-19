<?php
/**
 * @file
 * Contains EuropaEuVideoGalleryMigration class.
 */

define('YOUTUBE_PATH', 'youtube.com');

define('AVPORTAL_PATH', 'http://ec.europa.eu/avservices');

use Drupal\integration_consumer\Migrate\DocumentWrapperInterface;

/**
 * Class EuropaEuVideoGalleryMigration .
 */
class EuropaEuVideoGalleryMigration extends EuropaEuAbstractMigration {

  /**
   * {@inheritdoc}
   */
  public function __construct($arguments) {
    parent::__construct('video-gallery-items', $arguments);

    // Requires taxonomy terms migration to run before content migration.
    $this->setHardDependencies(array());

    $this->setMap(new MigrateSQLMap(
      $this->getMachineName(),
      $this->getSourceKey(),
      MigrateDestinationNode::getKeySchema()
    ));
    $this->setDestination(new MigrateDestinationNode('video_gallery', array('text_format' => 'full_html')));

    // Entity translation requires that both title fields are mapped.
    $this->addFieldMapping('title', 'document_title');
    $this->addFieldMapping('title_field', 'document_title');
    $this->addFieldMapping('body', 'body');

    // Mapping default language is necessary for correct translation handling.
    $this->addFieldMapping('language', 'default_language');

    $this->addFieldMapping('promote')->defaultValue(FALSE);
    $this->addFieldMapping('status')->defaultValue(NODE_PUBLISHED);

    $this->addUnmigratedDestinations(['field_video_items']);
  }

  /**
   * Implements prepare().
   */
  public function prepare(&$entity, $row) {
    if (isset($row->media)) {
      if (is_array($row->media)) {
        foreach ($row->media as $key => $url) {
          $file = $this->processFile($url, $row->title[$key], $entity->language, $row->image[$key]);
          $entity->field_video_items[$entity->language][$key]['fid'] = $file->fid;
          $entity->field_video_items[$entity->language][$key]['display'] = 1;
        }
      }
      else {
        $file = $this->processFile($row->media, $row->title, $entity->language, $row->image);
        $entity->field_video_items[$entity->language][0]['fid'] = $file->fid;
        $entity->field_video_items[$entity->language][0]['display'] = 1;
      }
    }
    $entity->path['pathauto'] = FALSE;
  }

  /**
   * Helper method to import videos.
   *
   * @param string $url
   *   File to process.
   * @param string $title
   *   Title of file.
   * @param string $language
   *   Target language.
   * @param bool $image
   *   Is image.
   *
   * @return bool|object|StdClass
   *   Returns processed file or false.
   */
  public function processFile($url, $title, $language, $image = FALSE) {
    if (strstr($url, YOUTUBE_PATH) !== FALSE) {
      $handler = new MediaInternetYouTubeHandler($url);
      $destination = $handler->parse($url);
      $file = file_uri_to_object($destination, TRUE);
    }
    elseif (strstr($url, AVPORTAL_PATH) !== FALSE) {
      $handler = new MediaInternetAvportalHandler($url);
      $file = $handler->getFileObject();
    }
    else {
      $handler = new MediaInternetFileHandler($url);
      $file = $handler->getFileObject();
    }

    if ($image) {
      $handler_image = new MediaInternetFileHandler($image);
      $file_image = $handler_image->getFileObject();
      if (!$file_image->fid) {
        file_save($file_image);
      }
    }

    if ($language == 'en') {
      $file->filename = $title;
      $file->filename_field[$language][0]['value'] = $title;
      if ($image) {
        $file->field_video_thumbnail[$language][0]['fid'] = $file_image->fid;
        $file->field_video_thumbnail[$language][0]['display'] = 1;
      }
    }
    else {
      $translation = array(
        'translate' => 0,
        'status' => 1,
        'source' => 'en',
        'language' => $language,
      );

      $values = [];
      $handler = entity_translation_get_handler('file', $file);
      $values['filename_field'][$language]['0']['value'] = $title;
      if ($image) {
        $values['field_video_thumbnail'][$language][0]['fid'] = $file_image->fid;
        $values['field_video_thumbnail'][$language][0]['display'] = 1;
      }
      $handler->setTranslation($translation, $values);
    }
    file_save($file);

    return $file;
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
        $new_entity->field_video_items = [];
        $this->getDestination()->prepare($new_entity, $this->sourceValues);

        $entity_type = $this->getDestination()->getEntityType();
        $bundle_name = $this->getDestination()->getBundle();

        $values = [];

        $values['title_field'] = $new_entity->title_field;
        $values['field_video_items'] = $new_entity->field_video_items;
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

        // Content based translation.
        if ($entity_type == 'node' && entity_translation_node_supported_type($entity->type)) {
          $translation['status'] = $entity->status;
          $translation['uid'] = $entity->uid;
          $translation['created'] = $entity->created;
          $translation['changed'] = $entity->changed;
        }

        // Add the new translation and store it.
        $translation_handler->setTranslation($translation, $values);

        // Preserve original language setting.
        $entity->field_language = $entity->language;
        $entity->language = $entity->translations->original;

        // Save entity.
        entity_save($entity_type, $entity);
      }
    }
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
    workbench_moderation_moderate($entity, 'published');
    if (entity_translation_enabled($this->getDestination()->getEntityType())) {
      $this->saveTranslations($entity, $source_row);
    }
  }

}
