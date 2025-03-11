/* GameData.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "GameData.h"

#include "audio/Audio.h"
#include "shader/BatchShader.h"
#include "CategoryList.h"
#include "Color.h"
#include "Command.h"
#include "ConditionsStore.h"
#include "Conversation.h"
#include "DataNode.h"
#include "DataWriter.h"
#include "Effect.h"
#include "Files.h"
#include "shader/FillShader.h"
#include "Fleet.h"
#include "shader/FogShader.h"
#include "text/FontSet.h"
#include "FormationPattern.h"
#include "Galaxy.h"
#include "GameAssets.h"
#include "GameEvent.h"
#include "Government.h"
#include "Hazard.h"
#include "image/ImageSet.h"
#include "Interface.h"
#include "shader/LineShader.h"
#include "image/MaskManager.h"
#include "Minable.h"
#include "Mission.h"
#include "audio/Music.h"
#include "News.h"
#include "Outfit.h"
#include "shader/OutlineShader.h"
#include "Person.h"
#include "Phrase.h"
#include "Planet.h"
#include "Plugins.h"
#include "shader/PointerShader.h"
#include "Politics.h"
#include "RenderBuffer.h"
#include "shader/RingShader.h"
#include "Ship.h"
#include "image/Sprite.h"
#include "image/SpriteSet.h"
#include "shader/SpriteShader.h"
#include "shader/StarField.h"
#include "StartConditions.h"
#include "System.h"
#include "TaskQueue.h"
#include "test/Test.h"
#include "test/TestData.h"
#include "UniverseObjects.h"

#include <algorithm>
#include <atomic>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>

using namespace std;

namespace {
	GameAssets assets;
	Set<Fleet> defaultFleets;
	Set<Government> defaultGovernments;
	Set<Planet> defaultPlanets;
	Set<System> defaultSystems;
	Set<Galaxy> defaultGalaxies;
	Set<Sale<Ship>> defaultShipSales;
	Set<Sale<Outfit>> defaultOutfitSales;
	Set<Wormhole> defaultWormholes;
	TextReplacements defaultSubstitutions;

	map<string, string> plugins;

	Politics politics;

	StarField background;

<<<<<<< HEAD
	vector<filesystem::path> sources;
	map<const Sprite *, shared_ptr<ImageSet>> deferred;
	map<const Sprite *, int> preloaded;
=======
	vector<string> sources;
>>>>>>> 0.10.10-editor-patched

	MaskManager maskManager;

	const Government *playerGovernment = nullptr;
	map<const System *, map<string, int>> purchases;

	ConditionsStore globalConditions;
<<<<<<< HEAD

	bool preventSpriteUpload = false;

	// Tracks the progress of loading the sprites when the game starts.
	std::atomic<bool> queuedAllImages = false;
	std::atomic<int> spritesLoaded = 0;
	std::atomic<int> totalSprites = 0;

	// List of image sets that are waiting to be uploaded to the GPU.
	mutex imageQueueMutex;
	queue<shared_ptr<ImageSet>> imageQueue;

	// Loads a sprite and queues it for upload to the GPU.
	void LoadSprite(TaskQueue &queue, const shared_ptr<ImageSet> &image)
	{
		queue.Run([image] { image->Load(); },
			[image] { image->Upload(SpriteSet::Modify(image->Name()), !preventSpriteUpload); });
	}

	void LoadSpriteQueued(TaskQueue &queue, const shared_ptr<ImageSet> &image);
	// Loads a sprite from the image queue, recursively.
	void LoadSpriteQueued(TaskQueue &queue)
	{
		if(imageQueue.empty())
			return;

		// Start loading the next image in the list.
		// This is done to save memory on startup.
		LoadSpriteQueued(queue, imageQueue.front());
		imageQueue.pop();
	}

	// Loads a sprite from the given image, with progress tracking.
	// Recursively loads the next image in the queue, if any.
	void LoadSpriteQueued(TaskQueue &queue, const shared_ptr<ImageSet> &image)
	{
		queue.Run([image] { image->Load(); },
			[image, &queue]
			{
				image->Upload(SpriteSet::Modify(image->Name()), !preventSpriteUpload);
				++spritesLoaded;

				// Start loading the next image in the queue, if any.
				lock_guard lock(imageQueueMutex);
				LoadSpriteQueued(queue);
			});
	}

	void LoadPlugin(TaskQueue &queue, const filesystem::path &path)
	{
		const auto *plugin = Plugins::Load(path);
		if(!plugin)
			return;

		if(plugin->enabled)
			sources.push_back(path);

		// Load the icon for the plugin, if any.
		auto icon = make_shared<ImageSet>(plugin->name);

		// Try adding all the possible icon variants.
		for(const string &extension : ImageBuffer::ImageExtensions())
		{
			filesystem::path iconPath = path / ("icon" + extension);
			if(Files::Exists(iconPath))
			{
				icon->Add(iconPath);
				break;
			}
		}
		for(const string &extension : ImageBuffer::ImageExtensions())
		{
			filesystem::path iconPath = path / ("icon@2x" + extension);
			if(Files::Exists(iconPath))
			{
				icon->Add(iconPath);
				break;
			}
		}

		if(!icon->IsEmpty())
		{
			icon->ValidateFrames();
			LoadSprite(queue, icon);
		}
	}
=======
>>>>>>> 0.10.10-editor-patched
}



future<void> GameData::BeginLoad(int options)
{
	// Initialize the list of "source" folders based on any active plugins.
<<<<<<< HEAD
	LoadSources(queue);

	if(!onlyLoadData)
	{
		queue.Run([&queue] {
			// Now, read all the images in all the path directories. For each unique
			// name, only remember one instance, letting things on the higher priority
			// paths override the default images.
			map<string, shared_ptr<ImageSet>> images = FindImages();

			// From the name, strip out any frame number, plus the extension.
			for(auto &it : images)
			{
				// This should never happen, but just in case:
				if(!it.second)
					continue;

				// Reduce the set of images to those that are valid.
				it.second->ValidateFrames();
				// For landscapes, remember all the source files but don't load them yet.
				if(ImageSet::IsDeferred(it.first))
					deferred[SpriteSet::Get(it.first)] = std::move(it.second);
				else
				{
					lock_guard lock(imageQueueMutex);
					imageQueue.push(std::move(std::move(it.second)));
					++totalSprites;
				}
			}
			queuedAllImages = true;
=======
	LoadSources();
>>>>>>> 0.10.10-editor-patched

	return assets.Load(sources, options);
}



void GameData::FinishLoading()
{
	// Store the current state, to revert back to later.
	defaultFleets = assets.objects.fleets;
	defaultGovernments = assets.objects.governments;
	defaultPlanets = assets.objects.planets;
	defaultSystems = assets.objects.systems;
	defaultGalaxies = assets.objects.galaxies;
	defaultShipSales = assets.objects.shipSales;
	defaultOutfitSales = assets.objects.outfitSales;
	defaultSubstitutions = assets.objects.substitutions;
	defaultWormholes = assets.objects.wormholes;
	playerGovernment = assets.objects.governments.Get("Escort");

	politics.Reset();
}



void GameData::CheckReferences()
{
	assets.objects.CheckReferences();
}



void GameData::LoadSettings()
{
	// Load the key settings.
	Command::LoadSettings(Files::Resources() / "keys.txt");
	Command::LoadSettings(Files::Config() / "keys.txt");
}



void GameData::LoadShaders()
{
	FontSet::Add(Files::Images() / "font/ubuntu14r.png", 14);
	FontSet::Add(Files::Images() / "font/ubuntu18r.png", 18);

	FillShader::Init();
	FogShader::Init();
	LineShader::Init();
	OutlineShader::Init();
	PointerShader::Init();
	RingShader::Init();
	SpriteShader::Init();
	BatchShader::Init();
	RenderBuffer::Init();

	background.Init(16384, 4096);
}



double GameData::GetProgress()
{
<<<<<<< HEAD
	double spriteProgress = 0.;
	if(queuedAllImages)
	{
		if(!totalSprites)
			spriteProgress = 1.;
		else
			spriteProgress = static_cast<double>(spritesLoaded) / totalSprites;
	}
	return min({spriteProgress, Audio::GetProgress(), objects.GetProgress()});
=======
	return assets.GetProgress();
>>>>>>> 0.10.10-editor-patched
}



bool GameData::IsLoaded()
{
	return GetProgress() == 1.;
}



// Begin loading a sprite that was previously deferred. Currently this is
// done with all landscapes to speed up the program's startup.
future<void> GameData::Preload(const Sprite *sprite)
{
	return assets.Preload(sprite);
}



// Get the list of resource sources (i.e. plugin folders).
<<<<<<< HEAD
const vector<filesystem::path> &GameData::Sources()
=======
const std::vector<std::string> &GameData::Sources()
>>>>>>> 0.10.10-editor-patched
{
	return sources;
}



// Get a reference to the UniverseObjects object.
UniverseObjects &GameData::Objects()
{
	return assets.objects;
}



// Revert any changes that have been made to the universe.
void GameData::Revert()
{
	assets.objects.fleets.Revert(defaultFleets);
	assets.objects.governments.Revert(defaultGovernments);
	assets.objects.planets.Revert(defaultPlanets);
	assets.objects.systems.Revert(defaultSystems);
	assets.objects.galaxies.Revert(defaultGalaxies);
	assets.objects.shipSales.Revert(defaultShipSales);
	assets.objects.outfitSales.Revert(defaultOutfitSales);
	assets.objects.substitutions.Revert(defaultSubstitutions);
	assets.objects.wormholes.Revert(defaultWormholes);
	for(auto &it : assets.objects.persons)
		it.second.Restore();

	politics.Reset();
	purchases.clear();
}



void GameData::SetDate(const Date &date)
{
	for(auto &it : assets.objects.systems)
		it.second.SetDate(date);
	politics.ResetDaily();
}



void GameData::ReadEconomy(const DataNode &node)
{
	if(!node.Size() || node.Token(0) != "economy")
		return;

	vector<string> headings;
	for(const DataNode &child : node)
	{
		if(child.Token(0) == "purchases")
		{
			for(const DataNode &grand : child)
				if(grand.Size() >= 3 && grand.Value(2))
					purchases[Systems().Get(grand.Token(0))][grand.Token(1)] += grand.Value(2);
		}
		else if(child.Token(0) == "system")
		{
			headings.clear();
			for(int index = 1; index < child.Size(); ++index)
				headings.push_back(child.Token(index));
		}
		else
		{
			System &system = *assets.objects.systems.Get(child.Token(0));

			int index = 0;
			for(const string &commodity : headings)
				system.SetSupply(commodity, child.Value(++index));
		}
	}
}



void GameData::WriteEconomy(DataWriter &out)
{
	out.Write("economy");
	out.BeginChild();
	{
		// Write each system and the commodity quantities purchased there.
		if(!purchases.empty())
		{
			out.Write("purchases");
			out.BeginChild();
			using Purchase = pair<const System *const, map<string, int>>;
			WriteSorted(purchases,
				[](const Purchase *lhs, const Purchase *rhs)
					{ return lhs->first->TrueName() < rhs->first->TrueName(); },
				[&out](const Purchase &pit)
				{
					// Write purchases for all systems, even ones from removed plugins.
					for(const auto &cit : pit.second)
						out.Write(pit.first->TrueName(), cit.first, cit.second);
				});
			out.EndChild();
		}
		// Write the "header" row.
		out.WriteToken("system");
		for(const auto &cit : GameData::Commodities())
			out.WriteToken(cit.name);
		out.Write();

		// Write the per-system data for all systems that are either known-valid, or non-empty.
		for(const auto &sit : GameData::Systems())
		{
			if(!sit.second.IsValid() && !sit.second.HasTrade())
				continue;

			out.WriteToken(sit.second.TrueName());
			for(const auto &cit : GameData::Commodities())
				out.WriteToken(static_cast<int>(sit.second.Supply(cit.name)));
			out.Write();
		}
	}
	out.EndChild();
}



void GameData::StepEconomy()
{
	// First, apply any purchases the player made. These are deferred until now
	// so that prices will not change as you are buying or selling goods.
	for(const auto &pit : purchases)
	{
		System &system = const_cast<System &>(*pit.first);
		for(const auto &cit : pit.second)
			system.SetSupply(cit.first, system.Supply(cit.first) - cit.second);
	}
	purchases.clear();

	// Then, have each system generate new goods for local use and trade.
	for(auto &it : assets.objects.systems)
		it.second.StepEconomy();

	// Finally, send out the trade goods. This has to be done in a separate step
	// because otherwise whichever systems trade last would already have gotten
	// supplied by the other systems.
	for(auto &it : assets.objects.systems)
	{
		System &system = it.second;
		if(!system.Links().empty())
			for(const Trade::Commodity &commodity : Commodities())
			{
				double supply = system.Supply(commodity.name);
				for(const System *neighbor : system.Links())
				{
					double scale = neighbor->Links().size();
					if(scale)
						supply += neighbor->Exports(commodity.name) / scale;
				}
				system.SetSupply(commodity.name, supply);
			}
	}
}



void GameData::AddPurchase(const System &system, const string &commodity, int tons)
{
	if(tons < 0)
		purchases[&system][commodity] += tons;
}



// Apply the given change to the universe.
void GameData::Change(const DataNode &node)
{
	assets.objects.Change(node);
}



// Update the neighbor lists and other information for all the systems.
// This must be done any time that a change creates or moves a system.
void GameData::UpdateSystems()
{
	assets.objects.UpdateSystems();
}



void GameData::AddJumpRange(double neighborDistance)
{
	assets.objects.neighborDistances.insert(neighborDistance);
}



// Re-activate any special persons that were created previously but that are
// still alive.
void GameData::ResetPersons()
{
	for(auto &it : assets.objects.persons)
		it.second.ClearPlacement();
}



// Mark all persons in the given list as dead.
void GameData::DestroyPersons(vector<string> &names)
{
	for(const string &name : names)
		assets.objects.persons.Get(name)->Destroy();
}



const Set<Color> &GameData::Colors()
{
	return assets.objects.colors;
}



const Set<Conversation> &GameData::Conversations()
{
	return assets.objects.conversations;
}



const Set<Effect> &GameData::Effects()
{
	return assets.objects.effects;
}



const Set<GameEvent> &GameData::Events()
{
	return assets.objects.events;
}



const Set<Fleet> &GameData::Fleets()
{
	return assets.objects.fleets;
}



const Set<FormationPattern> &GameData::Formations()
{
	return assets.objects.formations;
}



const Set<Galaxy> &GameData::Galaxies()
{
	return assets.objects.galaxies;
}



const Set<Government> &GameData::Governments()
{
	return assets.objects.governments;
}



const Set<Hazard> &GameData::Hazards()
{
	return assets.objects.hazards;
}



const Set<Interface> &GameData::Interfaces()
{
	return assets.objects.interfaces;
}



const Set<Minable> &GameData::Minables()
{
	return assets.objects.minables;
}



const Set<Mission> &GameData::Missions()
{
	return assets.objects.missions;
}



const Set<News> &GameData::SpaceportNews()
{
	return assets.objects.news;
}



const Set<Outfit> &GameData::Outfits()
{
	return assets.objects.outfits;
}



const Set<Sale<Outfit>> &GameData::Outfitters()
{
	return assets.objects.outfitSales;
}



const Set<Person> &GameData::Persons()
{
	return assets.objects.persons;
}



const Set<Phrase> &GameData::Phrases()
{
	return assets.objects.phrases;
}



const Set<Planet> &GameData::Planets()
{
	return assets.objects.planets;
}



const Set<Ship> &GameData::Ships()
{
	return assets.objects.ships;
}



const Set<Test> &GameData::Tests()
{
	return assets.objects.tests;
}



const Set<TestData> &GameData::TestDataSets()
{
	return assets.objects.testDataSets;
}



ConditionsStore &GameData::GlobalConditions()
{
	return globalConditions;
}



const Set<Sale<Ship>> &GameData::Shipyards()
{
	return assets.objects.shipSales;
}



const Set<System> &GameData::Systems()
{
	return assets.objects.systems;
}



const Set<Wormhole> &GameData::Wormholes()
{
	return assets.objects.wormholes;
}



const Government *GameData::PlayerGovernment()
{
	return playerGovernment;
}



Politics &GameData::GetPolitics()
{
	return politics;
}



const vector<StartConditions> &GameData::StartOptions()
{
	return assets.objects.startConditions;
}



const vector<Trade::Commodity> &GameData::Commodities()
{
	return assets.objects.trade.Commodities();
}



const vector<Trade::Commodity> &GameData::SpecialCommodities()
{
	return assets.objects.trade.SpecialCommodities();
}



// Custom messages to be shown when trying to land on certain stellar assets.objects.
bool GameData::HasLandingMessage(const Sprite *sprite)
{
	return assets.objects.landingMessages.count(sprite);
}



const string &GameData::LandingMessage(const Sprite *sprite)
{
	static const string EMPTY;
	auto it = assets.objects.landingMessages.find(sprite);
	return (it == assets.objects.landingMessages.end() ? EMPTY : it->second);
}



// Get the solar power and wind output of the given stellar object sprite.
double GameData::SolarPower(const Sprite *sprite)
{
	auto it = assets.objects.solarPower.find(sprite);
	return (it == assets.objects.solarPower.end() ? 0. : it->second);
}



double GameData::SolarWind(const Sprite *sprite)
{
	auto it = assets.objects.solarWind.find(sprite);
	return (it == assets.objects.solarWind.end() ? 0. : it->second);
}



// Get the map icon of the given stellar object sprite.
const Sprite *GameData::StarIcon(const Sprite *sprite)
{
	const auto it = objects.starIcons.find(sprite);
	return (it == objects.starIcons.end() ? nullptr : it->second);
}



// Strings for combat rating levels, etc.
const string &GameData::Rating(const string &type, int level)
{
	static const string EMPTY;
	auto it = assets.objects.ratings.find(type);
	if(it == assets.objects.ratings.end() || it->second.empty())
		return EMPTY;

	level = max(0, min<int>(it->second.size() - 1, level));
	return it->second[level];
}



// Collections for ship, bay type, outfit, and other categories.
const CategoryList &GameData::GetCategory(const CategoryType type)
{
	return assets.objects.categories[type];
}



const StarField &GameData::Background()
{
	return background;
}



void GameData::SetHaze(const Sprite *sprite, bool allowAnimation)
{
	background.SetHaze(sprite, allowAnimation);
}



const string &GameData::Tooltip(const string &label)
{
	static const string EMPTY;
	auto it = assets.objects.tooltips.find(label);
	// Special case: the "cost" and "sells for" labels include the percentage of
	// the full price, so they will not match exactly.
	if(it == assets.objects.tooltips.end() && !label.compare(0, 4, "cost"))
		it = assets.objects.tooltips.find("cost:");
	if(it == assets.objects.tooltips.end() && !label.compare(0, 9, "sells for"))
		it = assets.objects.tooltips.find("sells for:");
	return (it == assets.objects.tooltips.end() ? EMPTY : it->second);
}



string GameData::HelpMessage(const string &name)
{
	static const string EMPTY;
	auto it = assets.objects.helpMessages.find(name);
	return Command::ReplaceNamesWithKeys(it == assets.objects.helpMessages.end() ? EMPTY : it->second);
}



const map<string, string> &GameData::HelpTemplates()
{
	return assets.objects.helpMessages;
}



MaskManager &GameData::GetMaskManager()
{
	return maskManager;
}



const TextReplacements &GameData::GetTextReplacements()
{
	return assets.objects.substitutions;
}


const Set<string> &GameData::Music()
{
	return assets.music;
}



const SpriteSet &GameData::Sprites()
{
	return assets.sprites;
}


<<<<<<< HEAD
	vector<filesystem::path> globalPlugins = Files::ListDirectories(Files::GlobalPlugins());
	for(const auto &path : globalPlugins)
		if(Plugins::IsPlugin(path))
			LoadPlugin(queue, path);

	vector<filesystem::path> localPlugins = Files::ListDirectories(Files::UserPlugins());
	for(const auto &path : localPlugins)
		if(Plugins::IsPlugin(path))
			LoadPlugin(queue, path);
=======

const SoundSet &GameData::Sounds()
{
	return assets.sounds;
>>>>>>> 0.10.10-editor-patched
}



GameAssets &GameData::Assets()
{
<<<<<<< HEAD
	map<string, shared_ptr<ImageSet>> images;
	for(const auto &source : sources)
	{
		// All names will only include the portion of the path that comes after
		// this directory prefix.
		filesystem::path directoryPath = source / "images/";

		vector<filesystem::path> imageFiles = Files::RecursiveList(directoryPath);
		for(auto &path : imageFiles)
			if(ImageSet::IsImage(path))
			{
				ImageFileData data(path, directoryPath);

				shared_ptr<ImageSet> &imageSet = images[data.name];
				if(!imageSet)
					imageSet.reset(new ImageSet(data.name));
				imageSet->Add(std::move(data));
			}
	}
	return images;
=======
	return assets;
}



const Gamerules &GameData::GetGamerules()
{
	return assets.objects.gamerules;
>>>>>>> 0.10.10-editor-patched
}



void GameData::LoadSources()
{
	sources.clear();
	sources.push_back(Files::Resources());
}


// Thread-safe way to draw the menu background.
void GameData::DrawMenuBackground(Panel *panel)
{
	assets.objects.DrawMenuBackground(panel);
}
